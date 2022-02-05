#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void safe_system(const char *cmd) {
    int ret_val = system(cmd);
    if (ret_val != 0) {
        printf("failed %s!\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}


