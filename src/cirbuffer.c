#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "utils.h"
#include "cirbuffer.h"


_Bool space_in_buffer(struct cirbuffer* buf) {
    return !(buf->putfs + 1 == buf->takefs || (buf->putfs == BUFLEN - 1 && buf->takefs == 0));
}

void get_buffer(struct cirbuffer* buf, char* content) {
    if (!data_in_buffer(buf)) return;
    int takefs = buf->takefs, putfs = buf->putfs;
    if (takefs < putfs) {
        memcpy(content, buf->buf + takefs, putfs - takefs);
    }
    else {
        memcpy(content, buf->buf + takefs, BUFLEN - takefs);
        memcpy(content + BUFLEN - takefs, buf->buf, putfs);    
    }
}

int read_into_buffer(int fd, struct cirbuffer* buf) {
    int nread, nread2;
    if (!space_in_buffer(buf)) return 0;
    printf("    read_into_buffer start: takefs=%d, putfd=%d\n", buf->takefs, buf->putfs);
    if (buf->putfs + 1 < buf->takefs) {
        if ((nread = read(fd, buf->buf + buf->putfs, buf->takefs - buf->putfs - 1)) < 0)
            return -1;
        buf->putfs += nread;
        return nread;
    }
    if (buf->takefs == 0) {
        if ((nread = read(fd, buf->buf + buf->putfs, BUFLEN - buf->putfs - 1)) < 0)   
            return -1;
        buf->putfs += nread;
        return nread;
    }
    if ((nread = read(fd, buf->buf + buf->putfs, BUFLEN - buf->putfs)) < 0)
        return -1;
    buf->putfs = (buf->putfs + nread) % BUFLEN;
    if (buf->putfs == 0 && buf->takefs > 1) {
        if ((nread2 = read(fd, buf, buf->takefs - 1)) < 0)
            return -1;
        buf->putfs += nread2;
        return nread + nread2;
    }
    printf("    read_into_buffer end: takefs=%d, putfd=%d\n", buf->takefs, buf->putfs);
    return nread;
}

_Bool data_in_buffer(struct cirbuffer* buf) {
    return (buf->putfs != buf->takefs);
}

int write_from_buffer(int fd, struct cirbuffer* buf) {
    int nwrite, nwrite2;
    if (!data_in_buffer(buf)) return 0;
    printf("    write_from_buffer start: takefs=%d, putfd=%d\n", buf->takefs, buf->putfs);
    if (buf->takefs < buf->putfs) {
        if ((nwrite = write(fd, buf->buf + buf->takefs, buf->putfs - buf->takefs)) < 0)
            return -1;
        buf->takefs += nwrite;
        return nwrite;
    }
  
    if ((nwrite = write(fd, buf->buf + buf->takefs, BUFLEN - buf->takefs)) < 0)
        return -1;
    buf->takefs = (buf->takefs + nwrite) % BUFLEN;
    if (buf->takefs == 0 && buf->putfs > 0) {
        if ((nwrite2 = write(fd, buf, buf->putfs)) < 0)
            return -1;
        buf->takefs += nwrite2;
        return nwrite + nwrite;
    }
    printf("    write_from_buffer end: takefs=%d, putfd=%d\n", buf->takefs, buf->putfs);
    return nwrite;
}