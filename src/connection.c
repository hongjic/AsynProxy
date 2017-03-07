#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "utils.h"
#include "sockutils.h"
#include "connection.h"

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

int open_connection(int server, struct linkinfo *links, struct addrinfo *hostinfo, int *max_fd) {
    struct linkinfo *link;
    int position = -1, client, host;

    // create client connection
    if ((client = server_accept(server)) < 0)
        perr("Error accepting client");
    // assign link 
    for (int i = 0; i < MAX_CONN; ++i) {
        link = links + i;
        if (!link->active) {
            position = i;
            break;
        }
    }
    if (position < 0) {
        // send a message to client and close
        send(client, "max connections arrive, exit", 28, 0);
        close(client);
        return -1;
    }

    // create host connection
    if ((host = host_connect(hostinfo)) < 0) {
        close(client);
        perr("host_connect");
    }
    // make async
    if (make_async(host) < 0 || make_async(client) < 0) {
        close(host);
        close(client);
        perr("make_async");
    }
    // update max_fd
    if (host + 1 > *max_fd) *max_fd = host + 1;
    if (client + 1 > *max_fd) *max_fd = client + 1;

    link->server = host;
    link->client = client;
    link->active = 1;
    printf("Connection %d established.\n", position);

    return position;
err:
    return -2;
}

int close_connection(int fd, struct linkinfo *links, int *max_fd) {
    struct linkinfo *link;
    int i;

    for (i = 0; i < MAX_CONN; ++i) {
        link = links + i;
        if (link->active && (link->client == fd || link->server == fd))
            break;
    }

    // clear buffer and inactive
    link->req_buf.putfs = 0;
    link->req_buf.takefs = 0;
    link->res_buf.putfs = 0;
    link->res_buf.takefs = 0;

    // close socket
    send(link->client, "bye", 3, 0);
    close(link->client);
    close(link->server);
    link->client = 0;
    link->server = 0;
    link->active = 0;
    printf("Connectoin %d closed.\n", i);

    // update max_fd
    make_maxfd(links, max_fd);
    return i;
}

void make_maxfd(struct linkinfo *links, int *max_fd) {
    struct linkinfo* link;

    for (int i = 0; i < MAX_CONN; ++ i) {
        link = links + i;
        if (link->active) {
            if (link->client + 1 > *max_fd) *max_fd = link->client + 1;
            if (link->server + 1 > *max_fd) *max_fd = link->server + 1;
        }
    }
}