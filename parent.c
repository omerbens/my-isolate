
#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "util.h"
#include "child.h"

void write_to_file(const char *path, const char *line) {
    FILE *f = fopen(path, "w");
    if (NULL == f) {
        fprintf(stderr, "Error on opening file %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    unsigned long written = fwrite(line, 1, strlen(line), f);
    if (strlen(line) != written) {
        fprintf(stderr, "Error on writing to file %s\n%s", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fclose(f)) {
        fprintf(stderr, "Error on closing file %s\n%s", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
}


void setup_userns(pid_t pid) {
    char path[1000];
    char line[20];

    sprintf(path, "/proc/%d/uid_map", pid);
    sprintf(line, "0 1000 1000\n");
    write_to_file(path, line);

    sprintf(path, "/proc/%d/gid_map", pid);
    sprintf(line, "0 1000 1000\n");
    write_to_file(path, line);
}

void setup_netns(pid_t pid) {
    char line[1000];

    safe_system("sudo ip link add veth0 type veth peer name peer0");
    safe_system("sudo ip link set veth0 up");
    safe_system("sudo ip addr add 10.11.12.13/24 dev veth0");

    sprintf(line, "sudo ip link set peer0 netns /proc/%d/ns/net", pid);
    safe_system(line);
}


void isolate(int argc, char *argv[]) {
    struct info i;
    int fds[2];
    int flags;
    pid_t pid;

    if (pipe(fds) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    flags = SIGCHLD | \
            CLONE_NEWUSER | CLONE_NEWNET | CLONE_NEWUTS | \
            CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWPID;

    // the child stack grown downwards
    i.fds = fds;
    i.argv = argv;
    i.argc = argc;
    pid = clone(child_func, stack_child + STACK_SIZE, flags, &i);
    if (pid == -1) {
        fprintf(stderr, "clone: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    setup_userns(pid);
    setup_netns(pid);

    write(fds[1], &pid, sizeof(int));
    close(fds[1]);
    waitpid(pid, NULL, 0);
}

