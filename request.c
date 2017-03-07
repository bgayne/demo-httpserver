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


char * getLocalPath(char ** path, int len) {
    if(strncmp("/", *path, 1) == 0 && len == 1) {
        return "./index.html";
    }
    char * localPath = malloc(len + 2);
    memset(localPath, 0, len+2);
    *localPath = '.';
    return strncat(localPath, *path, len);

}

long int getFileLen(char * path, int len) {

    char * localPath = getLocalPath(&path, len);
    FILE * f;
    if((f = fopen(localPath, "r")) == NULL) {
        f = fopen("./404.html", "r");
    }
    fseek(f, 0L, SEEK_END);
    int testlen = ftell(f);
    fclose(f);
    printf("%d\n", testlen);
    return testlen;
}

int getFile(char * path, char ** buffer, long int * index, int fl) {

    char c;
    char * buffer_local = malloc(1);
    memset(buffer_local, 0, 1);

    static FILE * f;

    if(f == NULL)
        if((f = fopen(path, "r")) == NULL) f = fopen("./404.html", "r");

    int counter = 0;

    while(counter < 8096 && *index <= fl) {
        c = fgetc(f);
        buffer_local[counter] = c;
        buffer_local = realloc(buffer_local, ++counter + 1);
        (*index)++;
    }
    //printf("Read %d bytes this time, and %ld bytes total", counter, *index);
    *buffer = buffer_local;
    return counter;
}
