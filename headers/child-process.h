#ifndef JCUNIT_CHILD_PROCESS_H
#define JCUNIT_CHILD_PROCESS_H 1

#define RUN_MODE_CAPTURE_STDOUT (0)
#define RUN_MODE_CAPTURE_STDERR (1)

#define ERROR_CODE_NONE             (0)
#define ERROR_CODE_FILE_NOT_FOUND   (1)
#define ERROR_CODE_NOT_EXECUTABLE   (2)
#define ERROR_CODE_READ_CHILD_DATA  (3)

struct process_output
{
    char * buffer;
    unsigned int size;
    unsigned int len;
    unsigned int error_code;
};

void child_process_run(const char * path, char * argv[], struct process_output * output, unsigned int mode);

#endif /* JCUNIT_CHILD_PROCESS_H */
