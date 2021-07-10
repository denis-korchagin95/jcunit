#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "headers/child_process.h"

#define PIPE_INPUT_FD   (0)
#define PIPE_OUTPUT_FD  (1)

static int fd[2] = {0};

void child_process_run(const char * path, char * argv[], struct process_output * output, unsigned int mode)
{
    pipe(fd);

    pid_t pid = fork();

    if(pid < 0) {
        fprintf(stderr, "fork error!\n");
        exit(1);
    }

    int replace_fd = -1;

    switch (mode) {
        case RUN_MODE_CAPTURE_STDOUT:
            replace_fd = STDOUT_FILENO;
            break;
        case RUN_MODE_CAPTURE_STDERR:
            replace_fd = STDERR_FILENO;
            break;
    }

    if (replace_fd == -1) {
        fprintf(stderr, "unknown mode %u\n", mode);
        exit(1);
    }

    if (pid == 0) {
        dup2(fd[PIPE_OUTPUT_FD], replace_fd);

        close(fd[PIPE_OUTPUT_FD]);
        close(fd[PIPE_INPUT_FD]);

        int status = execv(path, argv);

        if (status != 0) {
            fprintf(stderr, "exec error\n");
            exit(1);
        }

        exit(0);
    }

    wait(NULL);

    close(fd[PIPE_OUTPUT_FD]);

    ssize_t nbytes = read(fd[PIPE_INPUT_FD], output->buffer, output->size);

    if (nbytes < 0) {
        fprintf(stderr, "IO error\n");
        exit(1);
    }

    output->len = (unsigned int)nbytes;
}

