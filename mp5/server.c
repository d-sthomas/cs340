#include "http.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>

void *client_thread(void *vptr) {
  int fd = *((int *)vptr);
  char buffer[4096];
  while (1) {
    ssize_t len = recv(fd, buffer, 4096, 0);
    if (len == -1) {
      printf("[%d]: socket closed\n", fd);
      break;
    } else if (len == 0) { 
      continue;
    }
    buffer[len] = '\0';
  
  
  HTTPRequest *req = (HTTPRequest *) malloc(sizeof(HTTPRequest));
  httprequest_parse_headers(req, buffer, len);
  const char *file[4096];
  strcat(file, "static/");
  if (strcmp(req->path, "/") == 0) {
    strcat(file, "index.html");
  } else strcat(file, req->path + 1);
 
  printf("file:%s\n", file);
  FILE *fp = fopen(file, "r");
  if (!fp) {
    sprintf(buffer, "%s\n", "404 Not Found");
    send(fd, buffer, strlen(buffer), 0);
  } else {
      fseek(fp, 0, SEEK_END);
      long flen = ftell(fp);
      rewind(fp);
      char data[flen];
      fread(data, flen, 1, fp);
      fclose(fp);

      const char *dot = strrchr(file, '.');
      char *header;
      if (strcmp(dot, ".png") == 0) header = "image/png";
      else if (strcmp(dot, ".html") == 0) header = "text/html";
      printf("header: %s\n", header);
      sprintf(buffer, "%s 200 OK\r\nContent-Length: %lu\r\nContent-Type: %s\r\n\r\n\0", req->version, flen, header);
      int l = strlen(buffer);
      l += (int) flen;
      memcpy(buffer+strlen(buffer), data, (int) flen);
      send(fd, buffer, l, 0);
      close(fd);
    }
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  printf("Binding to port %d. Visit http://localhost:%d/ to interact with your server!\n", port, port);

  // socket:
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // bind:
  struct sockaddr_in server_addr, client_address;
  memset(&server_addr, 0x00, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);  
  bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

  // listen:
  listen(sockfd, 10);

  // accept:
  socklen_t client_addr_len;
  while (1) {
    int *fd = malloc(sizeof(int));
    client_addr_len = sizeof(struct sockaddr_in);
    *fd = accept(sockfd, (struct sockaddr *)&client_address, &client_addr_len);
    printf("Client connected (fd=%d)\n", *fd);

    pthread_t tid;
    pthread_create(&tid, NULL, client_thread, fd);
    pthread_detach(tid);
  }

  return 0;
}