#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "util.h"


void setup_utsns() {
    int ret = sethostname("isolated", strlen("isolated"));
    if (0 != ret) {
        fprintf(stderr, "Error setting hostname %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void setup_mntns() {
    safe_system("mount -t proc proc /proc");
}

void user_setup_netns() {
    safe_system("ip link set lo up");
    safe_system("ip link set peer0 up");
    safe_system("ip addr add 10.11.12.14/24 dev peer0");
}

void user_setup_uid() {
    int ret_uid = setuid(0);
    int ret_gid = setgid(0);
    if (-1 == ret_uid || -1 == ret_gid) {
        fprintf(stderr, "Error setting uid/gid inside userns of child process -%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

char **format_args(int argc, char *argv[]) {
    char **program_args = malloc(sizeof(char *) * argc);
    if (NULL == program_args) {
        printf("failed malloc program_args");
        exit(EXIT_FAILURE);
    }

    // shift args on backward
    for (int i = 1; i < argc; i++) {
        program_args[i - 1] = argv[i];
    }

    // last one should be null
    program_args[argc] = NULL;
    return program_args;
}


int child_func(void *args) {
    struct info *info = (struct info *) args;
    int *fds = info->fds;
    pid_t pid;


    safe_system("echo -n \"##### old id:\"; id");

    read(fds[0], &pid, sizeof(int));
    close(fds[0]);
    printf("##### got \"signal\" from parent - continue\n");


    user_setup_uid();
    safe_system("echo -n \"##### new id:\"; id");


    setup_mntns();
    setup_utsns();
    user_setup_netns();

    // should be free - but because of execvp and exit on fail. that's fine
    char **program_args = format_args(info->argc, info->argv);

    for (int i = 0; i < info->argc; i++) {
        printf("#####     ARG %d %s\n", i, program_args[i]);
    }

    safe_system("echo \"##### my pid $$\"");
    safe_system("echo -n \"##### hostname: \"; hostname");
    safe_system("echo \"##### ping: \"; ping -c 1 10.11.12.13");

    printf("-------------------------\n");

    int ret = execvp(program_args[0], program_args);
    if (-1 == ret) {
        fprintf(stderr, "Error opening program -%s\n  %s\n", program_args[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
}

