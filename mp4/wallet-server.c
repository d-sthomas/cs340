#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "lib/wallet.h"
/*
  apt-get update
  apt-get install lsof
*/
// Your server must use only one global wallet:
wallet_t wallet;

void *client_communication_thread(void *vptr_fd) {
  int fd = *((int *)vptr_fd);
  char buffer[4096];

  while (1) {
    // recv message:
    ssize_t len = recv(fd, buffer, 4096, 0);
    if (len == -1) {
      printf("[%d]: socket closed\n", fd);
      break;
    } else if (len == 0) { 
      continue;
    }
    buffer[len] = '\0';
    char* buf = buffer; // casts to char*
    char *str; // used to read buf
    int count = 0; // keeps track of arguments read
    int get = 0; // flag to check for get
    int mod = 0; // flag to check for mod
    char name[len]; // stores name of resource
    int amt = 0; // stores amount for mod
    while ((str = strsep(&buf, " ")) != NULL) {
      if (count == 0) {
        if (strcmp(str, "MOD") == 0) mod = 1;
        else if (strcmp(str, "GET") == 0) get = 1;
        else { // EXIT command
          close(fd);
          return NULL;
        }
      }
      if (count == 1) {
        strcpy(name, str);
        name[strcspn(name, "\n")] = 0; // removes trailing \n
        if (get) break;
      }
      if (count == 2) {
        char tmp[6];
        strcpy(tmp, str);
        tmp[strcspn(tmp, "\n")] = 0; // removes trailing \n
        amt = atoi(tmp);
      }
      count++;
    } 
    if (get) {
      int data = wallet_get(&wallet, name);
      sprintf(buffer, "%d\n", data);
    }
    if (mod) {
      int data = wallet_change_resource(&wallet, name, amt);
      sprintf(buffer, "%d\n", data);
    }
    send(fd, buffer, strlen(buffer), 0); // send response:
  }
  return NULL;
}

void create_wallet_server(int port) {
  // Implement `create_wallet_server`
  // (You will need to use threads, which requires adding additional functions.)

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd <= 0) { perror("socket"); }
  printf("socket: returned fd=%d\n", sockfd);

  // bind:
  struct sockaddr_in server_addr;
  
  memset(&server_addr, 0x00, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);  
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    perror("bind");
  }
  printf("bind: binding fd=%d to port %d (:%d)\n", sockfd, port, port);

  // listen:
  if (listen(sockfd, 10) != 0) {
    perror("listen");
  }
  printf("listen: fd=%d is now listening for incoming connections\n", sockfd);

  // continue to accept new connections forever:
  while (1) {
    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    // accept:
    int *fd = malloc(sizeof(int));
    *fd = accept(sockfd, (struct sockaddr *)&client_address, &client_addr_len);
    
    char *ip = inet_ntoa(client_address.sin_addr);
    printf("accept: new client connected from %s, communication on fd=%d\n", ip, *fd);

    pthread_t tid;
    pthread_create(&tid, NULL, client_communication_thread, fd);
  }
}

int main(int argc, char* argv[]) {
  int c;
  int local_port = 34000;

  // Reads the (optional) command line argument:
  while((c = getopt(argc, argv, "p:")) != -1) {
    switch(c) {
      case 'p':
        if(optarg != NULL) {
          local_port = atoi(optarg);
        }
        break;
    }
  }

  // Calls `wallet_init`:
  wallet_init(&wallet);

  // Calls the `create_wallet_server` with the user-supplied port:
  printf("Launching wallet server on :%d\n", local_port);
  create_wallet_server(local_port);
  
}
