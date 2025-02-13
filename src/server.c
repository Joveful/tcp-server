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
  FILE *file = fopen("index.html", "r");
  size_t nread;
  char body[MAXDATASIZE];
  if (file) {
    while ((nread = fread(body, 1, sizeof(body), file)) > 0) {
      ;
    }
    fclose(file);
  }
  char response[strlen(header) + strlen(body)];
  strcpy(response, header);
  strcat(response, body);
  if (send(new_fd, response, strlen(response), 0) == -1)
    perror("send");
}

void parse_request(char *buf) {
  char *method = strtok(buf, " ");
  char *path = strtok(NULL, " ");
  char *protocol = strtok(NULL, "\r\n");
  printf("Method: %s\n", method);
  printf("Path: %s\n", path);
  printf("Protocol: %s\n", protocol);
  char *msg;
  while ((msg = strtok(NULL, "=")) != NULL) {
    if (strstr(msg, "fname") != NULL) {
      printf("POST: %s\n", strtok(NULL, "\r\n"));
    }
  }
  printf("\n");
}

int main() {
  printf("Server start\n");

  struct addrinfo hints, *res, *p;
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

  for (p = res; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) ==
        -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("bind");
      continue;
    }

    break;
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
      serve_website(new_fd);
      parse_request(&buf[0]);

      if (send(new_fd, "Hello, world!\n", 13, 0) == -1) {
        perror("send");
      }
      close(new_fd);
      exit(0);
    }
    close(new_fd);
  }
}
