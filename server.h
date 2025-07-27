#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

int server_contructor(serving_t* server);

#endif //SERVER_H
