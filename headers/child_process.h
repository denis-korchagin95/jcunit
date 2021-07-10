#ifndef JCUNIT_CHILD_PROCESS_H
#define JCUNIT_CHILD_PROCESS_H 1

#define RUN_MODE_CAPTURE_STDOUT (0)
#define RUN_MODE_CAPTURE_STDERR (1)

struct process_output
{
    char * buffer;
    unsigned int size;
    unsigned int len;
};

void child_process_run(const char * path, char * argv[], struct process_output * output, unsigned int mode);

#endif /* JCUNIT_CHILD_PROCESS_H */
