#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "server.h"

int server_launch(
    serving_t* server,
    int* out_socket_fd,
    char** request_buffer,
    size_t request_buffer_size)
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
    read(*out_socket_fd, request_buffer, request_buffer_size);
    return 0;
}

int main (void) {
    serving_t server = {
        .domain = AF_INET,
        .service = SOCK_STREAM,
        .protocol = 0,
        .interface = INADDR_ANY,
        .port = 8081,
        .backlog = 10
    };
    if (server_contructor(&server) > 0)
        exit(1);

    for (;;){
        // VERIFY GET AND POST REQUESTS
        static char buffer[30000] = {0};
        int connection_fd = -1;
        char* hello_msg = "HTTP/1.1 200 OK \r\n\r\n  Hello web. From C!";

        if(server_launch(&server, &connection_fd, (char**)&buffer, 30000)) {
            perror("Failed to launch server...\n");
            exit(1);
        };

        char* method = buffer;
        char* url = buffer + 5;
        *strchr(method, ' ') = 0;
        *strchr(url, ' ') = 0;

        printf("method: %s\nurl: %s\n", method, url);
        write(connection_fd, hello_msg, strlen(hello_msg));
        close(connection_fd);
    }
    close(server.socket);
    return 0;
}
