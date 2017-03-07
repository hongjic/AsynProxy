#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include "utils.h"
#include "sockutils.h"

#define LISTEN_PORT "9000"
#define BACKLOG     1024
#define BUFLEN      255

int main(int argc, char **argv) {
    int server, client;
    char buf[BUFLEN];
    int nread, nwritten, i;
    fd_set set;

    if((server = make_server(LISTEN_PORT, BACKLOG)) < 0)
        die("Cannot listen on port %s\n", LISTEN_PORT);

    // This is rather ugly code, but it demonstrates a straightforward
    // server (note that a lot of grunt work is done by the utility
    // library I wrote called sockutils (sockutils.h and sockutils.c),
    // which are also provided to you.
    //
    // In particular, note that this server terminates the connection
    // with the client after one request/response cycle. How would you
    // modify this code to allow the client to send arbitrarily many
    // requests during a single connection, terminating the connection
    // with the special input "quit"?

    if((client = server_accept(server)) < 0)
        die("Error accepting client");
    
    while(1) { // infinite loop, needed because we want the server to serve
               // clients repeatedly
        // You probably don't want to just kill your whole server
        // when you have a problem talking to a single client, so
        // we handle errors by closing the client and starting the
        // loop over, accepting the next incoming client.
        if((nread = read(client, buf, BUFLEN)) < 0) {
            fprintf(stderr, "Could not read from client\n");
            close(client);
            continue;
        }
        if (buf == "quit") {
            close(client);
            break;
        }

        for(i = 0; i < nread; ++i)
            buf[i] = toupper(buf[i]);
        
        if((nwritten = write(client, buf, nread)) < 0) {
            fprintf(stderr, "Could not write to client\n");
            close(client);
            continue;
        }
        
        // what about short writes?

        // close(client);
    }
    
    return 0;
}
