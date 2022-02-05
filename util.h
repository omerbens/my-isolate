//
// Created by omer on 05/02/2022.
//

#ifndef ISOLATE_UTIL_H
#define ISOLATE_UTIL_H


#define STACK_SIZE (1024*1024)
char stack_child[STACK_SIZE];

struct info {
    int argc;
    int *fds;
    char **argv;
};

void safe_system(const char *cmd);

#endif //ISOLATE_UTIL_H
