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
#include <dirent.h>

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

int serving_t_read_request (int socket_fd, Strpm* buffer) {
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
        bytes = recv(socket_fd, buffering.string, buffering.size, 0);
        Strpm_concat(buffer, &buffering);
    } while (bytes >= buffering.size);

    if (bytes < 0) {
        perror("ERROR: Failed to create request buffer\n");
        return 1;
    }

    return 0;
}

typedef struct serving_t_request serving_t_request;
typedef struct serving_t_response serving_t_response;

struct serving_t_response {
    int status;
    bool html;
    bool json;
    bool text;
    char* send;
};

int serving_t_parse_request (Strpm * from, serving_t_request * to) {
    Strpm headers = {0};
    Strpm body = {0};
    Strpm method = {0};
    Strpm url = {0};

    if (read_header_body(from, &headers, &body)) return 1;
    if (raed_method_url(&headers, &method, &url)) return 1;

    *to = (serving_t_request) {
        .url = url,
        .header = headers,
        .method = method
    };

    return 0;
}
int serving_t_create_response (serving_t_response* response, Strpm * buffer) {
    Strpm_auto_free tmp = {0};

    tmp.size += asprintf(&tmp.string, "HTTP/1.1 %d OK\r\n", response->status);

    if (response->html || response->text)
        tmp.size = asprintf(&tmp.string, "%sContent-Type: %s\r\n\r\n", tmp.string, "text/html");
    if (response->json)
        tmp.size = asprintf(&tmp.string, "%sContent-Type: %s\r\n\r\n", tmp.string, "application/json");

    tmp.size = asprintf(&tmp.string, "%s%s", tmp.string, response->send);

    if (tmp.size < 0) {
        perror("Unable to construct request\n");
        exit(EXIT_FAILURE);
    }

    Strpm_concat(buffer, &tmp);
    return 0;
}

int serving_t_endpoints_execute( serving_t_endpoints * endpoints, serving_t_request* request, serving_t_response* response) {
    int found = 1;
    for (size_t i = 0; i < endpoints->size; i++) {
        if ((Strpm_compare(&endpoints->methods[i], &request->method) == 0) && (Strpm_compare(&endpoints->paths[i], &request->url) == 0)) {
            endpoints->callbacks[i](request, response);
            found = 0;
        }
    }
    return found;
}

int serving_t_send_reponse(int connection_fd, serving_t_request * request, serving_t_response * response) {
    Strpm_auto_free string_response = {0};
    serving_t_create_response(response, &string_response);

    int sent = send(connection_fd, string_response.string, string_response.size, 0);
    printf("%s\n%s\n%s\n", Strpm_spit(&string_response), Strpm_spit(&request->url), Strpm_spit(&request->header));

    if (sent < 0) {
        return 1;
    }
    return 0;
};

int serving_t_run_server (serving_t* server_config, const int PORT) {
    int connection_fd = -1;
    server_config->port = PORT;
    if(serving_t_contructor(server_config)){
        return 1;
    }

    while (true)
    {
        serving_t_request request = {0}; // free request strings
        serving_t_response response = {0}; // free response strings
        Strpm_auto_free buffer = {0};

        if(serving_t_launch(server_config, &connection_fd)) {
            perror("ERROR: Failed to launch server...\n");
            break;
        };

        if (serving_t_read_request(connection_fd, &buffer)) {
            perror("ERROR: Read request failed\n");
            close(connection_fd);
            break;
        }

        if (serving_t_parse_request(&buffer, &request)) {
            perror("ERROR: Parse request failed\n");
            close(connection_fd);
            break;
        }

        if(serving_t_endpoints_execute(&server_config->endpoints, &request, &response)) {
            response = (serving_t_response) {
                .status = 404,
                .text = true,
                .send = "Not found\n",
            };
        }

        if (serving_t_send_reponse(connection_fd, &request, &response)) {
            perror("ERROR: Parse request failed\n");
            close(connection_fd);
            break;
        }

        close(connection_fd);
    };

    close(server_config->socket);
    return 0;
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

    *res = (serving_t_response) {
        .status = 200,
        .html = true,
        .send = Strpm_spit(&file_buffer)
    };

    fclose(file_pointer);
}

void post (serving_t_request * req, serving_t_response *res) {
    (void)(req);
    Strpm_auto_free response_text = {0};
    Strpm_init_after(&response_text, "This is the post...\n");

    *res = (serving_t_response) {
        .status = 200,
        .text = true,
        .send = Strpm_spit(&response_text)
    };
}

void serving_t_load_file (serving_t_request * req, serving_t_response *res) {
    (void)(req);
    (void)(res);

    Strpm_auto_free file_buffer = {0};
    char* file = Strpm_spit(&req->url);
    asprintf(&file, ".%s", file);

    FILE* file_pointer;
    if((file_pointer = fopen(file, "r")) == NULL)
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

    *res = (serving_t_response) {
        .status = 200,
        .html = true,
        .send = Strpm_spit(&file_buffer)
    };

    fclose(file_pointer);
}

int serving_t_static(serving_t * server, const char* directory) {
    (void)(server);
    struct dirent *entry;
    char* dire = NULL;
    asprintf(&dire, ".%s", directory);
    DIR *dir = opendir(dire);
    if (dir) {
        int count = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (count < 2) {
                count++;
                continue;
            }
            char* url;
            asprintf(&url, "%s/%s", directory, entry->d_name);
            serving_t_set(server, "GET", url, &serving_t_load_file);
        }
        closedir(dir);
    }
    return 0;
}


#define PORT 6969
int main (void) {
    serving_t server;
    serving_t_static(&server, "/src");
    serving_t_static(&server, "/lib");
    serving_t_set(&server, "GET", "/", &home); // create free mecanisms for the strings alocated for functions and methods
    serving_t_set(&server, "POST", "/", &post);
    return serving_t_run_server(&server, PORT);
}
