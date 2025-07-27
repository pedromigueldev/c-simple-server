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

int read_request (int* socket_fd, stringpm_t* buffer) {
    stringpm_t buffering = {
        .string = malloc(sizeof(char)*SERVING_PACKET_SIZE),
        .size = SERVING_PACKET_SIZE
    };

    int bytes = 1;
    do {
        bytes = recv(*socket_fd, buffering.string, buffering.size, 0);
        stringpm_t_concat(buffer, &buffering);
    } while (bytes >= buffering.size);

    return 0;
}

struct sigaction old_action;
void sigint_handler(int sig_no) {
    close(server.socket);
    write(STDOUT_FILENO, "\nClosing server with code: %d\n", sig_no);
    sigaction(SIGINT, &old_action, NULL);
    kill(0, SIGINT);
}

int main (void){
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &sigint_handler;
    sigaction(SIGINT, &action, &old_action);

    if (serving_t_contructor(&server))
        exit(1);

    for (;;){
        static stringpm_t buffer;
        static stringpm_t string_response;
        stringpm_t_init(&string_response, "HTTP/1.1 200 OK \r\n\r\nOlá sou Pedro Miguel\n");

        int connection_fd = -1;

        if(serving_t_launch(&server, &connection_fd)) {
            perror("Failed to launch server...\n");
            exit(1);
        };

        read_request(&connection_fd, &buffer);

        printf("%s", string_response.string);
        write(connection_fd, string_response.string, string_response.size);
        close(connection_fd);
    }
    close(server.socket);
    return 0;
}
