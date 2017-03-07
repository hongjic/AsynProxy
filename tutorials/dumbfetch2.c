#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "utils.h"
#include "sockutils.h"

#define BUFLEN 255

int make_async(int s) {
    int n;

    if ((n = fcntl(s, F_GETFL)) == -1 || fcntl(s, F_SETFL, n | O_NONBLOCK) == -1)
        perr("fcntl");
    n = 1;
    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &n, sizeof(n)) == -1)
        perr("setsockopt");
    return 0;

err:
    return -1;
}

int main(int argc, char **argv) {
    struct addrinfo *info;
    char *request = "GET /?page_id=72 HTTP/0.9\r\n\r\nGET /?page_id=72 HTTP/0.9\r\n\r\n";
    char response[BUFLEN];
    int host, n;

    if(argc != 3)
        die("Usage: %s server port\n", argv[0]);
    info = make_addrinfo(argv[1], argv[2]);
    
    if((host = host_connect(info)) < 0)
        pdie("host_connect");

    memset(response, 0, BUFLEN);
    make_async(host);
    // This improves dumbfetch by checking for errors from read and
    // handling arbitrarily long responses from the server (using
    // a loop that calls read() as many times as necessary).
    //
    // Note that we still haven't handled the "short write" problem
    // or checked for errors from the call to write(). Your code will
    // have to deal with those situations!

    fd_set r_ready, w_ready;
    FD_SET(host, &r_ready);
    FD_SET(host, &w_ready);
    printf("host = %d\n", host);
    int count,nwrite = 0;
    if (select(host + 1, NULL, &w_ready, NULL, NULL) < 0) 
        pdie("select");
    printf("write= %d\n", nwrite = write(host, request, strlen(request)));
    while (1) {
        if (select(host + 1, &r_ready, NULL, NULL, NULL) < 0)
            pdie("select");
        printf("aa");
        if (FD_ISSET(host, &r_ready)) {
            printf("bb");
            n = read(host, response, BUFLEN);
            printf("read = %d\n", n);
            printf("write = %d\n", nwrite);
            count += n;
            // while((n = read(host, response, BUFLEN)) > 0) {
            //     printf("%s", response);
            //     count += n;
            // }
            printf("count = %d\n", count);   
        }
        if(n < 0)
            pdie("read");
    }

    // write(host, request, strlen(request)); // could still have short writes!

    close(host);   

    free_addrinfo(info);
    
    return 0;
}
