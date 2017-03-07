#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFLEN 255

int main(int argc, char **argv) {
    char *s1   = "hello world";
    char  s2[] = "foo bar baz";
    char  buf[BUFLEN];
    int i;

    memset(buf, 0, BUFLEN);
    memcpy(buf, s2, 3);
    memcpy(buf + 3, s1 + 6, 5);

    printf("i is at %p\n", &i);

    printf("%s\n", buf);
    
    return 0;
}
