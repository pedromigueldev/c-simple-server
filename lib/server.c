#include <asm-generic/errno-base.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "./strpm.c"

#define SERVING_PACKET_SIZE 800

typedef struct serving_t serving_t;
typedef struct serving_t_endpoints serving_t_endpoints;
typedef struct serving_t_request serving_t_request;
typedef void(serving_t_callback)(serving_t_request* req, Strpm* res);

struct serving_t_endpoints {
    size_t size;
    size_t capacity;
    Strpm* methods;
    Strpm* paths;
    serving_t_callback** callbacks;
};

struct serving_t_request {
    Strpm* url;
    Strpm** params;
    Strpm** query;
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

static serving_t* __server;

int serving_t_launch(
    serving_t* server,
    int* out_socket_fd)
{
    int address_length = sizeof(server->address);
    printf("============ WAITING FOR CONNECTION ============\n");
    if((*out_socket_fd = accept(
            server->socket,
            (struct sockaddr*)&server->address,
            (socklen_t*)&address_length)) < 0)
    {
        perror("Failed to accept new connection...\n");
        return 1;
    }
    return 0;
}

struct sigaction old_action;
void sigint_handler(int sig_no);

int serving_t_contructor(serving_t* server) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &sigint_handler;
    sigaction(SIGINT, &action, &old_action);

    __server = server;

    server->address.sin_family = server->domain;
    server->address.sin_port = htons(server->port);
    server->address.sin_addr.s_addr = htonl(server->interface);

    if ((server->socket = socket(server->domain, server->service, server->protocol)) < 0){
        perror("Fail to connect to socket...\n");
        return 1;
    }

    const int enable = 1;
    if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("Setsockopt(SO_REUSEADDR) failed\n");
        return 1;
    }

    if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        perror("Setsockopt(SO_REUSEADDR) failed\n");
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

int serving_t_set(serving_t* server, const char* http_method, const char* enpoint, serving_t_callback* callback) {
    if (server->endpoints.capacity == 0 || server->endpoints.size == 0) {
        server->endpoints.capacity = 0;
        server->endpoints.size = 2;

        server->endpoints.methods = calloc(server->endpoints.size, sizeof(Strpm));
        server->endpoints.paths = calloc(server->endpoints.size, sizeof(Strpm));
        server->endpoints.callbacks = calloc(server->endpoints.size * sizeof(serving_t_callback), sizeof(serving_t_callback*));
    }

    if (server->endpoints.capacity == server->endpoints.size) {
        server->endpoints.size *= 2;
        server->endpoints.methods = realloc(server->endpoints.methods, sizeof(Strpm) * server->endpoints.size);
        server->endpoints.paths = realloc(server->endpoints.paths, sizeof(Strpm) * server->endpoints.size);
        server->endpoints.callbacks = (serving_t_callback **)realloc(server->endpoints.callbacks, server->endpoints.size * sizeof(serving_t_callback));
    }

    if (server->endpoints.methods == NULL || server->endpoints.paths == NULL) {
        perror("Error when allocating memory for methods");
        return 1;
    }

    Strpm_init(url, enpoint);
    Strpm_init(method, http_method);

    Strpm_init_after(&server->endpoints.methods[server->endpoints.capacity], http_method);
    Strpm_init_after(&server->endpoints.paths[server->endpoints.capacity], enpoint);
    server->endpoints.callbacks[server->endpoints.capacity] = callback;
    server->endpoints.capacity++;

    return 0;
}

struct sigaction old_action;
void sigint_handler(int sig_no) {
    close(__server->socket);
    write(STDOUT_FILENO, "\nClosing server with code: %d\n", sig_no);
    sigaction(SIGINT, &old_action, NULL);
    kill(0, SIGINT);
}
