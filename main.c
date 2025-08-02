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

int raed_method_url(Strpm* header, Strpm* method, Strpm* url) {
    Strpm_auto_free tmp;
    Strpm_concat(&tmp, header);

    char * token = strtok(tmp.string, " ");
    Strpm_init_after(method, token);
    token = strtok(NULL, " ");
    Strpm_init_after(url, token);
    return 0;
}

int read_header_body (Strpm* from, Strpm* header, Strpm* body) {
    Strpm_auto_free tmp;
    Strpm_concat(&tmp, from);

    char* b = strstr(tmp.string, "\r\n\r\n");
    char* h = tmp.string;
    h[b-h] = '\0';

    Strpm_init_after(header, h);
    Strpm_init_after(body, b+1);
    return 0;
}

int read_request (int* socket_fd, Strpm* buffer) {
    Strpm_auto_free buffering = {
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
        Strpm_concat(buffer, &buffering);
    } while (bytes >= buffering.size);

    return 0;
}

int serving_t_run_server (serving_t* server_config, int PORT) {
    int connection_fd = -1;

    if (PORT != 0)
        server_config->port = PORT;

    for (;;)
    {
        //create function to clear strings
        Strpm_auto_free buffer = {0};
        Strpm_auto_free string_response = {0};
        Strpm_auto_free headers = {0};
        Strpm_auto_free body = {0};
        Strpm_auto_free method = {0};
        Strpm_auto_free url = {0};

        Strpm_init_after(&string_response, "HTTP/1.1 200 OK \r\n\r\n");

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
            if (Strpm_compare(&server_config->endpoints.methods[i], &method) == 0 &&
                Strpm_compare(&server_config->endpoints.paths[i], &url) == 0)
            {
                server_config->endpoints.callbacks[i](&request, &string_response);
            }
        }
        write(connection_fd, string_response.string, string_response.size);
        close(connection_fd);
    };

    return close(server_config->socket);
}

void printIt (serving_t_request * req, Strpm *res) {
    Strpm_auto_free response_text = {0};
    Strpm_init_after(&response_text, "This is a response from the GET\n");

    Strpm_concat(&response_text, req->url);
    Strpm_concat(res, &response_text);
}

void testing2 (serving_t_request * req, Strpm *res) {
    Strpm_auto_free response_text = {0};
    Strpm_init_after(&response_text, "This is a response from the POST\n");

    Strpm_concat(&response_text, req->url);
    Strpm_concat(res, &response_text);
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
