#define BUFLEN 255

struct cirbuffer {
    char buf[BUFLEN];
    int putfs, takefs;
};

/**
    read from client socket into request buffer
    return -1 if error
    return 0 if there is no space,
    otherwise, return the number of bytes which were readed into buffer.
*/
int read_into_buffer(int fd, struct cirbuffer* buf);

/**
    write from buffer to server socket
    return -1 if error
    return 0 if write nothing,
    otherwise return the number of bytes which were written to the socket.
*/
int write_from_buffer(int fd, struct cirbuffer* buf);

_Bool space_in_buffer(struct cirbuffer* buf);

_Bool data_in_buffer(struct cirbuffer* buf);

void get_buffer(struct cirbuffer* buf, char* content);