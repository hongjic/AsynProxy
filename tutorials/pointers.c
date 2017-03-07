#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int i1 = 0;
    int i2 = 1;
    int *i3 = malloc(sizeof(int));
    int *i4 = malloc(sizeof(int));

    // In the program output, notice that the stack and heap grow towards
    // each other (you aren't guaranteed any particular ordering of addresses
    // in memory, so this isn't something you should depend on, but it's
    // an interesting observable phenomenon that should confirm what you
    // learned in class about how programs execute).

    printf("%p\n%p\n%p\n%p\n", &i1, &i2, &i3, &i4);

    return 0;
}
