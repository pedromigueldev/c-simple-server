#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "server.h"
#include "stringpm.h"

static serving_t server = {
    .domain = AF_INET,
    .service = SOCK_STREAM,
    .protocol = 0,
    .interface = INADDR_ANY,
    .port = 6969,
    .backlog = 10
};

int raed_method_url(stringpm_t* header, stringpm_t* method, stringpm_t* url) {
    stringpm_t_auto_free(tmp);
    stringpm_t_concat(&tmp, header);

    char * token = strtok(tmp.string, " ");
    stringpm_t_init(method, token);
    token = strtok(NULL, " ");
    stringpm_t_init(url, token);
    return 0;
}

int read_header_body (stringpm_t* from, stringpm_t* header, stringpm_t* body) {
    stringpm_t_auto_free(tmp);
    stringpm_t_concat(&tmp, from);

    char* b = strstr(tmp.string, "\r\n\r\n");
    char* h = tmp.string;
    h[b-h] = '\0';

    stringpm_t_init(header, h);
    stringpm_t_init(body, b+1);
    return 0;
}

int read_request (int* socket_fd, stringpm_t* buffer) {
    stringpm_t_auto_free(buffering) = {
        .string = malloc(sizeof(char)*SERVING_PACKET_SIZE),
        .size = SERVING_PACKET_SIZE
    };

    if (buffering.string == NULL) {
        perror("Error creating memory for internal buffer\n");
        return 1;
    }

    int bytes = 1;
    do {
        bytes = recv(*socket_fd, buffering.string, buffering.size, 0);
        stringpm_t_concat(buffer, &buffering);
    } while (bytes >= buffering.size);

    return 0;
}

int stringpm_t_compare (stringpm_t* first, stringpm_t* second) {
    if (first->size != second->size) {
        return 1;
    }
    for (int i = 0; i < first->size; i++) {
        if (first->string[i] != second->string[i]) {
            return 1;
        }
    }
    return 0;
}

int serving_t_run_server (serving_t* server_config, int PORT) {
    int connection_fd = -1;

    if (PORT != 0)
        server_config->port = PORT;

    for (;;)
    {
        //create function to clear strings
        stringpm_t_auto_free(buffer) = {0};
        stringpm_t_auto_free(string_response) = {0};
        stringpm_t_auto_free(headers) = {0};
        stringpm_t_auto_free(body) = {0};
        stringpm_t_auto_free(method) = {0};
        stringpm_t_auto_free(url) = {0};

        stringpm_t_init(&string_response, "HTTP/1.1 200 OK \r\n\r\nOlá sou Pedro Miguel");

        if(serving_t_launch(server_config, &connection_fd)) {
            perror("Failed to launch server...\n");
            exit(1);
        };

        read_request(&connection_fd, &buffer);
        read_header_body(&buffer, &headers, &body );
        raed_method_url(&headers, &method, &url);

        stringpm_t_concat(&string_response, &body);
        stringpm_t_concat(&string_response, &headers);
        stringpm_t_concat(&string_response, &(stringpm_t){.string = "\n", .size = 1}); //leek maybe
        stringpm_t_concat(&string_response, &method);
        stringpm_t_concat(&string_response, &url);


        for (size_t i = 0; i < server_config->endpoints.capacity; i++) {
            if (stringpm_t_compare(&server_config->endpoints.paths[i], &url) == 0) {
                server_config->endpoints.callbacks[i](&buffer, &string_response);
            }
        }

        write(connection_fd, string_response.string, string_response.size);
        close(connection_fd);
    }

    close(server.socket);
    return 0;
}

struct sigaction old_action;
void sigint_handler(int sig_no) {
    close(server.socket);
    write(STDOUT_FILENO, "\nClosing server with code: %d\n", sig_no);
    sigaction(SIGINT, &old_action, NULL);
    kill(0, SIGINT);
}

int serving_t_get(serving_t* server, const char* url, serving_t_callback* callback) {

    if (server->endpoints.capacity == 0 || server->endpoints.size == 0) {
        server->endpoints.capacity = 0;
        server->endpoints.size = 2;

        server->endpoints.methods = malloc(sizeof(stringpm_t) * server->endpoints.size);
        server->endpoints.paths = malloc(sizeof(stringpm_t) * server->endpoints.size);
        server->endpoints.callbacks = malloc(sizeof(stringpm_t) * server->endpoints.size);
    }

    if (server->endpoints.capacity == server->endpoints.size) {
        server->endpoints.size *= 2;
        server->endpoints.methods = realloc(server->endpoints.methods, sizeof(stringpm_t) * server->endpoints.size);
        server->endpoints.paths = realloc(server->endpoints.paths, sizeof(stringpm_t) * server->endpoints.size);
        server->endpoints.callbacks = malloc(sizeof(stringpm_t) * server->endpoints.size);
    }

    if (server->endpoints.methods == NULL || server->endpoints.paths == NULL) {
        perror("Error when allocating memory for methods");
        return 1;
    }

    stringpm_t u = {0};
    stringpm_t_init(&u, url);

    stringpm_t_concat(&server->endpoints.methods[server->endpoints.capacity], &(stringpm_t) {.size = 3, .string = "GET"});
    stringpm_t_concat(&server->endpoints.paths[server->endpoints.capacity], &u);
    server->endpoints.callbacks[server->endpoints.capacity] = callback;
    server->endpoints.capacity++;

    return 0;
}


void printIt (stringpm_t * req, stringpm_t *res) {
    (void)(req);
    (void)(res);
    printf("Olaaaaaaaaaaaaaaaaaaa\n");
}

void printIt2 (stringpm_t * req, stringpm_t *res) {
    (void)(req);
    (void)(res);
    printf("Pedro Miguel\n");
}

#define PORT 6969
int main (void){
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &sigint_handler;
    sigaction(SIGINT, &action, &old_action);

    if (serving_t_contructor(&server))
        return 1;

    serving_t_get(&server, "/get", &printIt);
    serving_t_get(&server, "/pet", &printIt2);
    return serving_t_run_server(&server, PORT);
}
