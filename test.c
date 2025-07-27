#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "server.h"

int server_launch(serving_t* server) {
    static char buffer[30000];
    int address_length = sizeof(server->address);
    int new_socket_fd;
    char* hello_msg = "HTTP/1.1 200 OK \r\n\r\n Hello web. From C!";

    printf("============ WAITING FOR CONNECTION ============\n");
    if((new_socket_fd = accept(
            server->socket,
            (struct sockaddr*)&server->address,
            (socklen_t*)&address_length)) < 0)
    {
        perror("Failed to accept new connection...\n");
        return 1;
    }

    read(new_socket_fd, buffer, 30000);
    printf("%s\n", buffer);
    write(new_socket_fd, hello_msg, strlen(hello_msg));

    close(new_socket_fd);
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

    for (;;) {
        server_launch(&server);
    }
}
