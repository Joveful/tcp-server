/*
 * TCP server
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "8080"
#define BACKLOG 20
#define MAXDATASIZE 1024

void serve_website(int new_fd) {
  char *header = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
  char *body = "<html><body><h1>Hello, world!</h1></body></html>";
  char response[strlen(header) + strlen(body)];
  strcpy(response, header);
  strcat(response, body);
  send(new_fd, response, strlen(response), 0);
}

int main() {
  printf("Server start\n");

  struct addrinfo hints, *res;
  // struct sockaddr_storage their_addr;
  int sockfd, new_fd;
  socklen_t sin_size;
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) ==
      -1) {
    perror("socket");
    return 1;
  }
  if ((rv = bind(sockfd, res->ai_addr, res->ai_addrlen)) != 0) {
    perror("bind");
    return 1;
  }

  freeaddrinfo(res);

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  printf("Server is listening on port %s\n", PORT);

  while (1) {
    char buf[MAXDATASIZE];
    sin_size = sizeof(struct sockaddr_storage);
    if ((new_fd = accept(sockfd, (struct sockaddr *)&res, &sin_size)) == -1) {
      perror("accept");
      continue;
    }

    printf("Server: got connection from %s\n",
           inet_ntoa(((struct sockaddr_in *)&res)->sin_addr));

    if (!fork()) { // child process
      close(sockfd);
      if (recv(new_fd, buf, MAXDATASIZE - 1, 0) == -1) {
        perror("recv");
      }
      // write(1, buf, strlen(buf));
      serve_website(new_fd);
      if (send(new_fd, "Hello, world!", 13, 0) == -1) {
        perror("send");
      }
      close(new_fd);
      exit(0);
    }
    close(new_fd);
  }
}
