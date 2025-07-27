#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include "server.h"


int server_contructor(serving_t* server) {
    server->address.sin_family = server->domain;
    server->address.sin_port = htons(server->port);
    server->address.sin_addr.s_addr = htonl(server->interface);

    if ((server->socket = socket(server->domain, server->service, server->protocol)) < 0){
        perror("Fail to connect to socket...\n");
        return 1;
    }

    if ((bind(server->socket, (struct sockaddr*)&server->address, sizeof(server->address))) < 0) {
        perror("Failed to bind socket...\n");
        return 1;
    }

    if ((listen(server->socket, server->backlog)) < 0){
        perror("Failed to start listening...\n");
        return 1;
    }
    return 0;
};
