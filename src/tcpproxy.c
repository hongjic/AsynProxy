#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "utils.h"
#include "sockutils.h"
#include "connection.h"

#define BACKLOG 1024

int main(int argc, char **argv) {
    // declare
    int proxy_server, n_ready, max_fd, len;
    struct addrinfo *hostinfo;
    fd_set r_ready, w_ready;
    struct linkinfo links[MAX_CONN];

    // initialization
    if (argc != 4) 
        die("Usage: %s server port\n", argv[0]);
    hostinfo = make_addrinfo(argv[1], argv[2]);
    char *LISTEN_PORT = argv[3];
    printf("listen = %s\n", LISTEN_PORT);
    if ((proxy_server = make_server(LISTEN_PORT, BACKLOG)) < 0) {
        die("Cannot listen on port %s\n", LISTEN_PORT);
    }
    printf("proxy_server = %d\n", proxy_server);
    max_fd = proxy_server + 1;
    memset(links, 0, sizeof(links));

    int cc = 0;
    while (cc == 0) {
        printf("----------------loop----------------\n");
        FD_ZERO(&r_ready);
        FD_ZERO(&w_ready);
        FD_SET(proxy_server, &r_ready);
        int count = 0;
        for (int i = 0; i < MAX_CONN; ++i)
            if (links[i].active) {
                if (space_in_buffer(&links[i].req_buf)) {
                    FD_SET(links[i].client, &r_ready);
                    count ++;
                }
                if (data_in_buffer(&links[i].res_buf)) {
                    FD_SET(links[i].client, &w_ready);
                    count ++;
                }
                if (space_in_buffer(&links[i].res_buf)) {
                    FD_SET(links[i].server, &r_ready);
                    count ++;
                }
                if (data_in_buffer(&links[i].req_buf)) {
                    FD_SET(links[i].server, &w_ready);
                    count ++;
                }
            }
        printf("%d possible tasks for select.\n", count);
        if ((n_ready = select(max_fd, &r_ready, &w_ready, NULL, NULL)) < 0)
            pdie("select");
        // there is no timeout configuration. only exit when client send "quit"
        printf("There are %d tasks ready.\n", n_ready);
        // check if new connections come in
        if (FD_ISSET(proxy_server, &r_ready)) {
            printf("new connection!\n");
            -- n_ready;
            // create new connection
            int position, newc, news;
            if ((position = open_connection(proxy_server, links, hostinfo, &max_fd)) < 0) {
                if (position == -1) printf("Active clients reached upper limit.\n");
                else printf("Error establishing connection.\n");
            }
        }

        for (int i = 0; n_ready > 0 && i < MAX_CONN; ++i)    
            if (links[i].active) {
                if (FD_ISSET(links[i].client, &r_ready)) {
                    printf("reading request from client.\n");
                    -- n_ready;
                    // read the request
                    if ((len = read_into_buffer(links[i].client, &links[i].req_buf)) < 0) {
                        printf("Can not read request from client.\n");
                        close_connection(links[i].client, links, &max_fd);
                    }
                    printf("Read %d bytes from client.\n", len);
                    char req[BUFLEN];
                    get_buffer(&links[i].req_buf, req);
                    if (strncmp(req, "quit", 4) == 0) {
                        close_connection(links[i].client, links, &max_fd);
                    }
                }
                if (FD_ISSET(links[i].server, &w_ready)) {
                    printf("writting request to host.\n");
                    -- n_ready;
                    // write the request
                    if (write_from_buffer(links[i].server, &links[i].req_buf) < 0) {
                        printf("Can not write request to host.\n");
                        close_connection(links[i].server, links, &max_fd);
                    }
                }
                if (FD_ISSET(links[i].server, &r_ready)) {
                    printf("reading response from host.\n");
                    -- n_ready;
                    // read the response
                    if ((len = read_into_buffer(links[i].server, &links[i].res_buf)) < 0) {
                        printf("Can not read response from host.\n");
                        close_connection(links[i].server, links, &max_fd);
                    }
                    printf("Read %d bytes from host.\n", len);
                }
                if (FD_ISSET(links[i].client, &w_ready)) {
                    printf("writting response to client.\n");
                    -- n_ready;
                    // write the response
                    if (write_from_buffer(links[i].client, &links[i].res_buf) < 0) {
                        printf("Can not write response to client.\n");
                        close_connection(links[i].client, links, &max_fd);
                    }
                }
            }
  
    }

    free_addrinfo(hostinfo);
    return 0;

}