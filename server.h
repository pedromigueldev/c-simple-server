#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "stringpm.h"

#define SERVING_PACKET_SIZE 800

typedef struct {
    int domain;
    int service;
    int protocol;
    u_long interface;
    int port;
    int backlog;
    struct sockaddr_in address;
    int socket;
    void (*launch)(void);
} serving_t;

int serving_t_contructor(serving_t* server);
int serving_t_launch(serving_t* server,int* out_socket_fd);

#endif //SERVER_H
