#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "utils.h"
#include "sockutils.h"

#define BUFLEN 255

int main(int argc, char **argv) {
    struct addrinfo *info;
    char *request = "GET /?page_id=72 HTTP/0.9\r\n\r\n";
    char response[BUFLEN];
    int host;
    char arr[] = {};

    if(argc != 3)
        die("Usage: %s server port\n", argv[0]);
    info = make_addrinfo(argv[1], argv[2]);
    
    if((host = host_connect(info)) < 0)
        pdie("host_connect");

    memset(response, 0, BUFLEN);

    // The following code has problems, at least the following:
    // 
    // (1) write() is allowed to write fewer bytes than requested
    // (2) BUFLEN may not be enough space to hold the entire response
    // (3) there is no error checking (write and read return < 0 on error)
    
    write(host, request, strlen(request));
    read(host, response, BUFLEN);
    printf("%s\n", response);

    close(host);      
    free_addrinfo(info);
    
    return 0;
}
