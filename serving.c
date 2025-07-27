#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "server.h"

typedef struct {
    char* string;
    uint16_t size;
} string_t;

int string_t_concat (string_t* to, string_t* from) {
    if (to->size <= 0) {
        to->string = malloc(sizeof(char)*from->size);
        to->size = from->size;
        for (int i = 0; i < from->size; i++) {
            to->string[i] = from->string[i];
        }
        return 0;
    }

    int old_size = to->size;
    to->size = old_size + from->size;
    to->string = realloc(to->string, to->size);

    for (int i = 0; i < from->size; i++)
        to->string[i + old_size] = from->string[i];

    return 0;
}

int server_launch(
    serving_t* server,
    int* out_socket_fd,
    string_t* buffer){

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

    #define PACKET_SIZE 30
    string_t buffering = {
        .string = malloc(sizeof(char)*PACKET_SIZE),
        .size = PACKET_SIZE
    };

    do {
      int bytes_received = recv(*out_socket_fd, buffering.string, PACKET_SIZE, 0);
      if (bytes_received <= 0) {
        break;
      }
      string_t_concat(buffer, &buffering);
    } while (1);
    return 0;
}

int main (void) {
    serving_t server = {
        .domain = AF_INET,
        .service = SOCK_STREAM,
        .protocol = 0,
        .interface = INADDR_ANY,
        .port = 8080,
        .backlog = 10
    };
    if (server_contructor(&server) > 0)
        exit(1);

    for (;;){
        // VERIFY GET AND POST REQUESTS
        static string_t buffer = {0};

        int connection_fd = -1;
        char* hello_msg = "HTTP/1.1 200 OK \r\n\r\n  Hello web. From C!";

        if(server_launch(&server, &connection_fd, &buffer)) {
            perror("Failed to launch server...\n");
            exit(1);
        };

        printf("%s\n", buffer.string);

        write(connection_fd, hello_msg, strlen(hello_msg));
        close(connection_fd);
    }
    close(server.socket);
    return 0;
}
