#include "request_handler.h"
#include "dir_list.h"
#include "str_builder.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int parse_request_line(char *line, struct http_request_line *request_line) {
  int num_parsed = sscanf(line, "%s %s %s", request_line->method,
                          request_line->path, request_line->protocol);
  return num_parsed == 3;
}

int parse_headers(char *header_lines, struct http_header *request_headers) {
  int header_count = 0;
  char *line = strtok(header_lines, "\r\n");

  while (line != NULL && header_count < MAX_HEADERS) {
    if (strlen(line) == 0) {
      break;
    }

    if (sscanf(line, "%[^:]: %[^\r\n]", request_headers[header_count].key,
               request_headers[header_count].value)) {
      header_count++;
    }

    line = strtok(NULL, "\r\n");
  }

  return header_count;
}

int parse_body(char *body_start, struct http_request *request) {
  if (request->header_count == 0) {
    return 0;
  }

  int content_length = 0;

  for (int i = 0; i < request->header_count; i++) {
    if (strcasecmp(request->http_header[i].key, "content-length") == 0) {
      content_length = atoi(request->http_header[i].value);
      break;
    }
  }

  if (content_length == 0) {
    return 0;
  }

  request->body = malloc(content_length + 1);
  if (request->body) {
    strncpy(request->body, body_start, content_length);
    request->body[content_length] = '\0';
  }

  return content_length;
}

int parse_request(char *raw_request, struct http_request *request) {
  char *cur_pos = raw_request;
  char *line_end = strstr(cur_pos, "\r\n");

  if (line_end == NULL) {
    return -1;
  }

  *line_end = '\0';
  if (parse_request_line(cur_pos, &request->request_line) == 0) {
    return -1;
  }

  cur_pos = line_end + 2;
  line_end = strstr(cur_pos, "\r\n\r\n");
  if (line_end == NULL) {
    return -1;
  }

  *line_end = '\0';
  request->header_count = parse_headers(cur_pos, request->http_header);

  cur_pos = line_end + 4;
  parse_body(cur_pos, request);
  return 0;
}

void handle_internal_server_error(int sockfd) {
  const char *response =
      "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n"
      "<html><body><h1>500 Internal Server Error</h1><p>Failed to render "
      "directory listings.</p></body></html>";
  if (write(sockfd, response, strlen(response)) < 0) {
    error("ERROR writing to socket");
  }
}

void handle_unsupported_request(int sockfd) {
  const char *response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: "
                         "text/html\r\n\r\n<html><body><h1>405 Method Not "
                         "Allowed</h1></body></html>";
  if (write(sockfd, response, strlen(response)) < 0) {
    error("ERROR writing to socket");
  }
}

void handle_dir_listing(char *dir, int sockfd) {
  struct string_builder sb;
  struct dir_list dl;

  if (init_string_builder(&sb, 16)) {
    goto error;
  }

  if (append_string(
          &sb, "HTTP/1.1 200 OK\r\nContent-Type: "
               "text/html\r\n\r\n<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML "
               "4.01//EN\" "
               "\"http://www.w3.org/TR/html4/strict.dtd\"><html><head><meta "
               "http-equiv=\"Content-Type\" content=\"text/html; "
               "charset=utf-8\"><title>Directory listing for ") ||
      append_string(&sb, dir) ||
      append_string(&sb, "</title></head><body><h1>Directory listing for ") ||
      append_string(&sb, dir) || append_string(&sb, "</h1><hr>")) {
    goto error;
  }

  init_dir_list(&dl);
  int read_dir_status = read_dir(dir, &dl);
  if (read_dir_status != 0) {
    free_string_builder(&sb);
    free_dir_list(&dl);
    handle_internal_server_error(sockfd);
    return;
  }

  if (dl.count > 0) {
    if (append_string(&sb, "<ul>")) {
      goto error;
    }
    for (int i = 0; i < dl.count; i++) {
      if (append_string(&sb, "<li><a href=\"") ||
          append_string(&sb, dl.files[i]) || append_string(&sb, "\">") ||
          append_string(&sb, dl.files[i]) || append_string(&sb, "</a></li>")) {
        goto error;
      }
    }
    if (append_string(&sb, "</ul>")) {
      goto error;
    }
  }

  if (append_string(&sb, "<hr></body></html>")) {
    goto error;
  }

  const char *response = get_string(&sb);
  if (write(sockfd, response, strlen(response)) < 0) {
    perror("ERROR writing to socket");
    goto error;
  }

  free_string_builder(&sb);
  free_dir_list(&dl);
  return;

error:
  free_string_builder(&sb);
  free_dir_list(&dl);
  handle_internal_server_error(sockfd);
}

void handle_request(struct http_request *request, int sockfd) {
  struct stat buffer;

  char *path = request->request_line.path;
  if (path[0] == '/') {
    path++;
  }

  if (path[0] == '\0') {
    path = "/";
  }

  int status = stat(path, &buffer);
  if (status != 0) {
    handle_unsupported_request(sockfd);
  } else if (S_ISREG(buffer.st_mode)) {
    handle_unsupported_request(sockfd);
  } else if (S_ISDIR(buffer.st_mode)) {
    handle_dir_listing(request->request_line.path, sockfd);
  } else {
    handle_unsupported_request(sockfd);
  }

  if (request->body) {
    free(request->body);
  }
}
