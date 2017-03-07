int setResponse(char ** header, int * len);
int setContentType(char ** header, char * path, int * len);
int setContentLength(char ** header, int * header_len, int content_len);
int setContentLength(char ** header, int * header_len, int content_len);
int endHeaders(char ** header, int * len);
int scanCRLF(char * buffer, int size);
char *** buildHeaderTokens(char * buffer, int headers_len);
