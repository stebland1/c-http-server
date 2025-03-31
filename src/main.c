#include "request_handler.h"
#include "utils.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080

int main(void) {
  int sockfd, newsockfd;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  char buffer[1024];
  int n;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("ERROR opening socket");
  }

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    error("ERROR on binding");
  }

  listen(sockfd, 10);
  clilen = sizeof(cli_addr);

  printf("Server listening on port %d...\n", PORT);

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) {
      error("ERROR on accept");
    }

    bzero(buffer, 1024);
    n = read(newsockfd, buffer, 1023);
    if (n < 0) {
      error("ERROR reading from socket");
    }
    printf("Request: %s\n", buffer);

    struct http_request request;
    parse_request(buffer, &request);
    char *http_response = handle_request(&request);

    if (request.body) {
      free(request.body);
    }

    n = write(newsockfd, http_response, strlen(http_response));
    if (n < 0) {
      error("ERROR writing to socket");
    }

    close(newsockfd);
  }

  close(sockfd);
}
