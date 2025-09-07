#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "headers/child-process.h"
#include "headers/errors.h"

#define PIPE_COUNT (2)

#define PIPE_STDOUT (0)
#define PIPE_STDERR (1)

#define PIPE_INPUT_FD   (0)
#define PIPE_OUTPUT_FD  (1)

static int pipes[PIPE_COUNT][2] = {0};

void child_process_run(const char * path, char * argv[], struct process_output * output, unsigned int mode)
{
    pipe(pipes[PIPE_STDOUT]);
    pipe(pipes[PIPE_STDERR]);

    pid_t pid = fork();

    if(pid < 0) {
        jcunit_fatal_error("Cannot fork the child process: \"%s\"!", strerror(errno));
    }

    int pipe_index;

    switch (mode) {
        case RUN_MODE_CAPTURE_STDOUT:
            pipe_index = PIPE_STDOUT;
            break;
        case RUN_MODE_CAPTURE_STDERR:
            pipe_index = PIPE_STDERR;
            break;
        default:
            jcunit_fatal_error("The unknown mode: %u", mode);
    }

    if (pid == 0) {
        dup2(pipes[PIPE_STDOUT][PIPE_OUTPUT_FD], STDOUT_FILENO);
        dup2(pipes[PIPE_STDERR][PIPE_OUTPUT_FD], STDERR_FILENO);

        close(pipes[PIPE_STDOUT][PIPE_OUTPUT_FD]);
        close(pipes[PIPE_STDOUT][PIPE_INPUT_FD]);
        close(pipes[PIPE_STDERR][PIPE_OUTPUT_FD]);
        close(pipes[PIPE_STDERR][PIPE_INPUT_FD]);

        execv(path, argv);

        exit(0);
    }

    wait(NULL);

    close(pipes[PIPE_STDOUT][PIPE_OUTPUT_FD]);
    close(pipes[PIPE_STDERR][PIPE_OUTPUT_FD]);

    ssize_t nbytes = read(pipes[pipe_index][PIPE_INPUT_FD], output->buffer, output->size);

    close(pipes[PIPE_STDOUT][PIPE_INPUT_FD]);
    close(pipes[PIPE_STDERR][PIPE_INPUT_FD]);

    if (nbytes < 0) {
        output->error_code = ERROR_CODE_READ_CHILD_DATA;
        return;
    }

    output->len = (unsigned int)nbytes;
}
