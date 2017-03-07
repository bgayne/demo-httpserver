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

#define SERVER_PORT_DEFAULT "8306"

#include "response.h"
#include "request.h"
#include "headers.h"

int sendall(int socket, char * response, int len) {
    int sent = 0;
    while((sent += send(socket, response, len, 0)) != -1) {
        if(sent >= len) return sent;
    }
    return -1;
}

int respond(int sockid, char * buffer, int headers_len) {


    char * headers = malloc(headers_len);
    memcpy(headers, buffer, headers_len);
    char *** headerTokens = buildHeaderTokens(headers, headers_len);

    char * resp_headers = malloc(1);
    int response_len = 0;
    long int data_len = getFileLen(headerTokens[0][1], strnlen(headerTokens[0][1], 1000));

    setResponse(&resp_headers, &response_len);
    setContentType(&resp_headers, headerTokens[0][1], &response_len);
    setContentLength(&resp_headers, &response_len, data_len);
    endHeaders(&resp_headers, &response_len);

    sendall(sockid, resp_headers, response_len);

    char * data;
    unsigned long int index = 0;
    char * localPath = getLocalPath(&headerTokens[0][1], strnlen(headerTokens[0][1], 1000));
    while(index <= data_len) {
        int readTotal = getFile(localPath, &data, &index, data_len);
        sendall(sockid, data, readTotal);
        free(data);
    }

    return 0;

}
