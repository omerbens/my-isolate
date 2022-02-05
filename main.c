#include <stdio.h>
#include "parent.h"

/*
 * Parts of this code was taken from sources given in homework
 * "Digging into Linux namespaces" -> and PoC mentioned `https://github[.]com/mihailkirov/namespaces/tree/main/PoC`
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: isolate PROGRAM [ARGS...]\n");
        return -1;
    }
    isolate(argc, argv);
    return 0;
}