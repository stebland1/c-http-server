#include "request_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
