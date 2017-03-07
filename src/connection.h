#include "cirbuffer.h"

#define MAX_CONN 10

struct linkinfo {
    int client;
    int server;
    struct cirbuffer req_buf, res_buf;
    _Bool active;
};

/**
    make a file descriptor ready for asynchronous I/O
*/
int make_async(int s);

/**
    try to add a new client/server connection to the linkinfo list.
    return -2 if connection failure
    return -1 if linkinfo list is full
    otherwise, return the position of the new connection in the linkinfo list.
*/
int open_connection(int server, struct linkinfo *links, struct addrinfo *hostinfo, int *max_fd);


/**
    close a connection by a fd(client/server). update max_fd
    return the position released.
*/
int close_connection(int fd, struct linkinfo *links, int *max_fd);

/**
    update max_fd by traversing.
*/
void make_maxfd(struct linkinfo *links, int *max_fd);