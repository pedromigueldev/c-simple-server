#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "stringpm.h"

#define SERVING_PACKET_SIZE 800


typedef struct serving_t serving_t;
typedef struct serving_t_endpoints serving_t_endpoints;
typedef void(serving_t_callback)(stringpm_t* req, stringpm_t* res);


struct serving_t_endpoints {
    size_t size;
    size_t capacity;
    stringpm_t* methods;
    stringpm_t* paths;
    serving_t_callback** callbacks;
};

struct serving_t {
    int domain;
    int service;
    int protocol;
    u_long interface;
    int port;
    int backlog;
    struct sockaddr_in address;
    int socket;
    void (*launch)(void);
    serving_t_endpoints endpoints;
};


int serving_t_contructor(serving_t* server);
int serving_t_launch(serving_t* server,int* out_socket_fd);

#endif //SERVER_H
