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

int setResponse(char ** header, int * len) {
    char * response = "HTTP/1.1 200 OK\r\n\0";
    *len = strnlen(response, 200);

    *header = realloc(*header, *len + 1);
    memset(*header, 0, *len + 1);
    memcpy(*header, response, *len + 1);

    return 0;

}

int setContentType(char ** header, char * path, int * len) {
    int index = 0;
    char c;

    while((c = (*(path + index))) != '\0') {
        if(c == '.') {
            index++;
            break;
        }
        index++;
    }
    if(c == '\0') return -1;

    char * contentType;

    if(strncmp(path + index, "html", 4) == 0)
        contentType = "Content-Type:text/html\r\n\0"; // The null terminator is for strnlen purposes
    else if(strncmp(path + index, "js", 2) == 0)
        contentType = "Content-Type:text/javascript\r\n\0";
    else if(strncmp(path + index, "css", 3) == 0)
        contentType = "Content-Type:text/css\r\n\0";
    else if(strncmp(path + index, "jpeg", 4) == 0 || strncmp(path + index, "jpg", 3) == 0)
        contentType = "Content-Type:image/jpeg\r\n\0";
    else if(strncmp(path + index, "bmp", 3) == 0)
        contentType = "Content-Type:image/bmp\r\n\0";
    else
        contentType = "Content-Type:application/octet-stream\r\n\0"; //let the browser figure it out.


    int lineLength= strnlen(contentType, 100);
    *header = realloc(*header, *len + lineLength);
    memset(*header + *len, 0, lineLength);
    memcpy(*header + *len, contentType, lineLength);
    *len += lineLength;
    return 0;
}

int setContentLength(char ** header, int * header_len, int content_len) {
    char * contentLength = malloc(2048);
    int lineLength = sprintf(contentLength, "Content-Length: %d\r\n", content_len);
    *header = realloc(*header, *header_len + lineLength);
    memcpy(*header + *header_len, contentLength, lineLength);
    *header_len += lineLength;
    return 0;
}

int endHeaders(char ** header, int * len) {
    *header = realloc(*header, *len+2);
    memset(*header+*len, 0, 2);
    memcpy(*header+*len, "\r\n", 2);
    *len += 2;
    return 0;
}



/*
This function looks for the end of standard HTTP headers, CLRF on an empty line.
It only looks for a line feed in the preceding line instead of the standard CLRF because some
browsers, /Chrome/, have taken it upon themselves to not follow the common standard

Turns out that it just uses a new line character instead. There's an afternoon I'm never getting back.

This exists so we know when recv() has finished sending the full headers. It uses
a three byte window to check for a new line character and an empty CRLF line.

*/

int scanCRLF(char * buffer, int size) {

    char * cursor = buffer;
    int line_len = 0;
    if(size >= 3) {
        while(line_len <= (size - 3)) {
            if(strncmp(cursor, "\n\r\n", 3) == 0) {
                return line_len+1;
            }
            line_len++;
            cursor++;
        }
    }

    return -1;
}


/*
Here we're trying to build easily parsable tokens from our headers.
The structure of our headers will look something like this when they're done

                     | HTTP/1.1 | Blahblahblah | Somethingelse || <- Pointer (char*)
                     | /        | Accept       | Action        || <- Pointer (char*)
                     | GET      |              |               || ...
  Pointer(char***)-> |Index 1   | Index 2      | Index 3       || ...

                        ^Pointer   ^Pointer
                        (char**)   (char**)        ...

We always treat index one as a special case, tokenizing it against a space rather than
a colon like the rest of the headers.

If len(index one) != 3, we return an error.
If the first entry of index one does not equal one of the main http verbs (get/post/put/delete)
we return an error.
*/

char *** buildHeaderTokens(char * buffer, int headers_len) {

    int header_count = 0;
    int index = 0;
    char c;

    char * headerline;
    int hl_index = 0;

    char *** headers = malloc(1); //lord forgive me

    while(index < headers_len) {

        headerline = malloc(1); //preventing realloc weirdness.

        while((c = *buffer) != '\n') { //low rent getline()
            if(c == '\r') {
                *buffer = '\0';
                buffer++;
                index++;
                continue;
            }
            headerline = realloc(headerline, sizeof(char*) * (++hl_index));
            *(headerline + (hl_index - 1)) ^= 0; //memset 0
            headerline[hl_index - 1] = c;
            index++;
            buffer++;
        }

        *(buffer++) = '\0';

        hl_index = 0;
        //printf("%s", headerline);
        char ** header_tokens = malloc(sizeof(char*));
        char * h_tokens = strtok(headerline, (header_count == 0) ? " " : ":");
        int h_tokens_index = 1;

        header_tokens[h_tokens_index - 1] = h_tokens;
        header_tokens = realloc(header_tokens, sizeof(char*) * (++h_tokens_index));


        while((h_tokens = strtok(NULL, (header_count == 0) ? " " : ":")) != NULL) {
            //printf("%s", h_tokens);
            header_tokens[h_tokens_index - 1] = h_tokens;
            header_tokens = realloc(header_tokens, sizeof(char*) * (++h_tokens_index));
        } //I've created a monster.

        headers = realloc(headers, sizeof(char**) * (++header_count));
        headers[header_count - 1] = header_tokens;
    }

    return headers;

}
