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
#include "stringpm.h"

int main (void) {
    serving_t server = {
        .domain = AF_INET,
        .service = SOCK_STREAM,
        .protocol = 0,
        .interface = INADDR_ANY,
        .port = 8080,
        .backlog = 10
    };
    if (serving_t_contructor(&server) > 0)
        exit(1);

    for (;;){
        // VERIFY GET AND POST REQUESTS
        static stringpm_t buffer = {0};

        int connection_fd = -1;
        char* hello_msg = "HTTP/1.1 200 OK \r\n\r\n  Hello web. From C!";

        if(serving_t_launch(&server, &connection_fd, &buffer)) {
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
