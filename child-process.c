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

int child_process_run(
    const char * path,
    char * argv[],
    struct process_output * stdout_output,
    struct process_output * stderr_output
) {
    pid_t pid;
    int status;
    ssize_t nbytes;

    if (pipe(pipes[PIPE_STDOUT]) < 0) {
        jcunit_fatal_error("Cannot create stdout pipe: \"%s\"!", strerror(errno));
    }
    if (pipe(pipes[PIPE_STDERR]) < 0) {
        jcunit_fatal_error("Cannot create stderr pipe: \"%s\"!", strerror(errno));
    }

    pid = fork();

    if(pid < 0) {
        jcunit_fatal_error("Cannot fork the child process: \"%s\"!", strerror(errno));
    }

    if (pid == 0) {
        dup2(pipes[PIPE_STDOUT][PIPE_OUTPUT_FD], STDOUT_FILENO);
        dup2(pipes[PIPE_STDERR][PIPE_OUTPUT_FD], STDERR_FILENO);

        close(pipes[PIPE_STDOUT][PIPE_OUTPUT_FD]);
        close(pipes[PIPE_STDOUT][PIPE_INPUT_FD]);
        close(pipes[PIPE_STDERR][PIPE_OUTPUT_FD]);
        close(pipes[PIPE_STDERR][PIPE_INPUT_FD]);

        execv(path, argv);

        exit(127);
    }

    close(pipes[PIPE_STDOUT][PIPE_OUTPUT_FD]);
    close(pipes[PIPE_STDERR][PIPE_OUTPUT_FD]);

    /* read both pipes before waitpid to avoid deadlock */
    nbytes = read(pipes[PIPE_STDOUT][PIPE_INPUT_FD], stdout_output->buffer, stdout_output->size);
    if (nbytes < 0) {
        stdout_output->error_code = ERROR_CODE_READ_CHILD_DATA;
        stdout_output->len = 0;
    } else {
        stdout_output->len = (unsigned int)nbytes;
    }

    nbytes = read(pipes[PIPE_STDERR][PIPE_INPUT_FD], stderr_output->buffer, stderr_output->size);
    if (nbytes < 0) {
        stderr_output->error_code = ERROR_CODE_READ_CHILD_DATA;
        stderr_output->len = 0;
    } else {
        stderr_output->len = (unsigned int)nbytes;
    }

    close(pipes[PIPE_STDOUT][PIPE_INPUT_FD]);
    close(pipes[PIPE_STDERR][PIPE_INPUT_FD]);

    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}
