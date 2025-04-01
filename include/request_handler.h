#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#define MAX_HEADER_LEN 256
#define MAX_PATH_LEN 512
#define MAX_HEADERS 100

struct http_request_line {
  char method[10];
  char path[MAX_PATH_LEN];
  char protocol[10];
};

struct http_header {
  char key[MAX_HEADER_LEN];
  char value[MAX_HEADER_LEN];
};

struct http_request {
  struct http_request_line request_line;
  struct http_header http_header[MAX_HEADERS];
  int header_count;
  char *body;
};

int parse_request_line(char *line, struct http_request_line *request_line);
int parse_body(char *body_start, struct http_request *request);
int parse_headers(char *header_lines, struct http_header *request_headers);
int parse_request(char *raw_request, struct http_request *request);

void handle_request(struct http_request *request, int sockfd);
void handle_unsupported_request(int sockfd);
void handle_dir_listing(char *dir, int sockfd);
void handle_internal_server_error(int sockfd);

#endif
