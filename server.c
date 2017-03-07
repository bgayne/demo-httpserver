#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_PORT_DEFAULT "8000"

#include "response.h"
#include "request.h"
#include "headers.h"

int server_main(int argc, char * argv[]) {

    struct addrinfo hints, *results, *cursor;

    int socket_main;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if(getaddrinfo("localhost", SERVER_PORT_DEFAULT, &hints, &results) == -1)
        perror("getaddrinfo");
    for(cursor = results; cursor != NULL; cursor = cursor->ai_next) {
        if((socket_main = socket(cursor->ai_family, cursor->ai_socktype, cursor->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }
        if(bind(socket_main, cursor->ai_addr, cursor->ai_addrlen) == -1) {
            perror("bind");
            continue;
        }
        break;
    }
    if(cursor == NULL) {
        fprintf(stderr, "Couldn't start the server.\n");
    }

    while(1) {
        listen(socket_main, 20);
        struct sockaddr_storage * addr = malloc(sizeof(struct sockaddr_storage));
        memset(addr, 0, sizeof(struct sockaddr_storage));
        socklen_t addr_len = sizeof(struct sockaddr_storage);

        int client_socket = accept(socket_main, (struct sockadrr*)addr, &addr_len);

        int pid = fork();

        if(!pid) {
            char * headers = malloc(1);
            int headers_len = 0;
            char * data = malloc(4096);
            memset(data, 0, 4096);
            int recv_total;
            int count = 0;
            while((recv_total = recv(client_socket, data, 4096, 0)) != 0) {

                int tmp = headers_len;
                headers_len += recv_total;
                headers = realloc(headers, headers_len);

                memcpy(headers + tmp, data, recv_total);
                printf("%s", data);
                memset(data, 0, 4096);

                int line_len;
                if((line_len = scanCRLF(headers, headers_len)) != -1) {
                    respond(client_socket, headers, line_len);
                    break;
                }

            }
            exit(1);
        }
        close(client_socket);

    }



}
