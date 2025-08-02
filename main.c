#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "lib/server.c"

int raed_method_url(stringpm_t* header, stringpm_t* method, stringpm_t* url) {
    stringpm_t_auto_free tmp;
    stringpm_t_concat(&tmp, header);

    char * token = strtok(tmp.string, " ");
    stringpm_t_init_after(method, token);
    token = strtok(NULL, " ");
    stringpm_t_init_after(url, token);
    return 0;
}

int read_header_body (stringpm_t* from, stringpm_t* header, stringpm_t* body) {
    stringpm_t_auto_free tmp;
    stringpm_t_concat(&tmp, from);

    char* b = strstr(tmp.string, "\r\n\r\n");
    char* h = tmp.string;
    h[b-h] = '\0';

    stringpm_t_init_after(header, h);
    stringpm_t_init_after(body, b+1);
    return 0;
}

int read_request (int* socket_fd, stringpm_t* buffer) {
    stringpm_t_auto_free buffering = {
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
        stringpm_t_auto_free buffer = {0};
        stringpm_t_auto_free string_response = {0};
        stringpm_t_auto_free headers = {0};
        stringpm_t_auto_free body = {0};
        stringpm_t_auto_free method = {0};
        stringpm_t_auto_free url = {0};

        stringpm_t_init_after(&string_response, "HTTP/1.1 200 OK \r\n\r\n");

        if(serving_t_launch(server_config, &connection_fd)) {
            perror("Failed to launch server...\n");
            exit(1);
        };

        read_request(&connection_fd, &buffer);
        read_header_body(&buffer, &headers, &body );
        raed_method_url(&headers, &method, &url);

        serving_t_request request = {
            .url = &url,
        };

        for (size_t i = 0; i < server_config->endpoints.capacity; i++) {
            if (stringpm_t_compare(&server_config->endpoints.methods[i], &method) == 0 &&
                stringpm_t_compare(&server_config->endpoints.paths[i], &url) == 0)
            {
                server_config->endpoints.callbacks[i](&request, &string_response);
            }
        }
        write(connection_fd, string_response.string, string_response.size);
        close(connection_fd);
    };

    return close(server_config->socket);
}

int serving_t_set(serving_t* server, const char* http_method, const char* enpoint, serving_t_callback* callback) {
    if (server->endpoints.capacity == 0 || server->endpoints.size == 0) {
        server->endpoints.capacity = 0;
        server->endpoints.size = 2;

        server->endpoints.methods = calloc(server->endpoints.size, sizeof(stringpm_t));
        server->endpoints.paths = calloc(server->endpoints.size, sizeof(stringpm_t));
        server->endpoints.callbacks = calloc(server->endpoints.size * sizeof(serving_t_callback), sizeof(serving_t_callback*));
    }

    if (server->endpoints.capacity == server->endpoints.size) {
        server->endpoints.size *= 2;
        server->endpoints.methods = realloc(server->endpoints.methods, sizeof(stringpm_t) * server->endpoints.size);
        server->endpoints.paths = realloc(server->endpoints.paths, sizeof(stringpm_t) * server->endpoints.size);
        server->endpoints.callbacks = (serving_t_callback **)realloc(server->endpoints.callbacks, server->endpoints.size * sizeof(serving_t_callback));
    }

    if (server->endpoints.methods == NULL || server->endpoints.paths == NULL) {
        perror("Error when allocating memory for methods");
        return 1;
    }

    stringpm_t_init(url, enpoint);
    stringpm_t_init(method, http_method);

    stringpm_t_init_after(&server->endpoints.methods[server->endpoints.capacity], http_method);
    stringpm_t_init_after(&server->endpoints.paths[server->endpoints.capacity], enpoint);
    server->endpoints.callbacks[server->endpoints.capacity] = callback;
    server->endpoints.capacity++;

    return 0;
}

void printIt (serving_t_request * req, stringpm_t *res) {
    stringpm_t_auto_free response_text = {0};
    stringpm_t_init_after(&response_text, "This is a response from the GET\n");

    stringpm_t_concat(&response_text, req->url);
    stringpm_t_concat(res, &response_text);
}

void testing2 (serving_t_request * req, stringpm_t *res) {
    stringpm_t_auto_free response_text = {0};
    stringpm_t_init_after(&response_text, "This is a response from the POST\n");

    stringpm_t_concat(&response_text, req->url);
    stringpm_t_concat(res, &response_text);
}

#define PORT 6969
int main (void){

    static serving_t server = {
        .domain = AF_INET,
        .service = SOCK_STREAM,
        .protocol = 0,
        .interface = INADDR_ANY,
        .port = 6969,
        .backlog = 10
    };

    if (serving_t_contructor(&server))
        return 1;

    serving_t_set(&server, "GET", "/get", &printIt);
    serving_t_set(&server, "POST", "/get", &testing2);

    return serving_t_run_server(&server, PORT);
}
