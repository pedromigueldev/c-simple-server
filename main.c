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
#include "lib/serving.h"
#include "lib/strpm.h"

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

typedef struct serving_t_request serving_t_request;
typedef struct serving_t_response serving_t_response;

struct body {
    int status;
    char* send;
};

struct serving_t_response {
    struct body body;
};

int serving_t_run_server (serving_t* server_config, const int PORT) {
    int connection_fd = -1;
    server_config->port = PORT;
    if(serving_t_contructor(server_config)){
        return 1;
    }

    for (;;)
    {
        //create function to clear strings instead of recreating and freeing every request
        Strpm_auto_free buffer = {0};
        Strpm_auto_free string_response = {0};
        Strpm_auto_free headers = {0};
        Strpm_auto_free body = {0};
        Strpm_auto_free method = {0};
        Strpm_auto_free url = {0};

        Strpm_init_after(&string_response, "HTTP/1.1 200 OK\nContent-Type: text/html\r\n\r\n");

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
        serving_t_response response = {0};


        for (size_t i = 0; i < server_config->endpoints.capacity; i++) {
            if ((Strpm_compare(&server_config->endpoints.methods[i], &method) == 0) && (Strpm_compare(&server_config->endpoints.paths[i], &url) == 0))
                server_config->endpoints.callbacks[i](&request, &response);
        }

        if (response.body.send != NULL) {
            Strpm_auto_free tmp = {0};
            Strpm_init_after(&tmp, response.body.send);
            Strpm_concat(&string_response, &tmp);
        }

        send(connection_fd, string_response.string, string_response.size, 0);

        printf("%s\n%s\n%s\n", Strpm_spit(&string_response), Strpm_spit(&url), Strpm_spit(&headers));

        close(connection_fd);
    };

    return close(server_config->socket);
}

void home (serving_t_request * req, serving_t_response *res) {
    (void)(req);
    Strpm_auto_free file_buffer = {0};

    FILE* file_pointer;
    if((file_pointer = fopen("./src/index.html", "r")) == NULL)
    {
       printf("Error!");
       exit(1);
    }

    if (fseek(file_pointer, 0L, SEEK_END) == 0) {
        long bufsize = ftell(file_pointer);
        if (bufsize == -1) { /* Error */ }

        file_buffer.string = malloc(sizeof(char) * (bufsize + 1));

        if (fseek(file_pointer, 0L, SEEK_SET) != 0) { /* Error */ }

        size_t newLen = fread(file_buffer.string, sizeof(char), bufsize, file_pointer);
        file_buffer.size = newLen;

        if ( ferror( file_pointer ) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            file_buffer.string[newLen++] = '\0';
        }
    }

    res->body = (struct body) {
        .status=200,
        .send = Strpm_spit(&file_buffer)
    };

    fclose(file_pointer);
}

void post (serving_t_request * req, serving_t_response *res) {
    (void)(req);
    Strpm_auto_free response_text = {0};
    Strpm_init_after(&response_text, "This is the post...\n");

    res->body = (struct body) {
        .status=200,
        .send = Strpm_spit(&response_text)
    };
}


#define PORT 6969
int main (void){
    serving_t server;
    serving_t_set(&server, "GET", "/", &home);
    serving_t_set(&server, "POST", "/", &post);
    return serving_t_run_server(&server, PORT);
}
