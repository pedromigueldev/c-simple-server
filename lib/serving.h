#ifndef SERVING_H
#define SERVING_H

#include "./strpm.h"
#include <netinet/in.h>
#include <sys/types.h>

#define SERVING_PACKET_SIZE 800

typedef struct serving_t serving_t;
typedef struct serving_t_endpoints serving_t_endpoints;
typedef struct serving_t_request serving_t_request;
typedef struct serving_t_response serving_t_response;
typedef void (serving_t_callback)(serving_t_request* req, serving_t_response* res);

struct serving_t_endpoints {
    size_t size;
    size_t capacity;
    Strpm* methods;
    Strpm* paths;
    serving_t_callback** callbacks;
};

struct serving_t_request {
    Strpm url;
    Strpm method;
    Strpm header;
    Strpm* params;
    Strpm* query;
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

int serving_t_launch(serving_t* server, int* out_socket_fd);
int serving_t_contructor(serving_t* server);
int serving_t_set(serving_t* server, const char* http_method, const char* enpoint, serving_t_callback* callback);
#endif
