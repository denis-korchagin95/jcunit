#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "headers/errors.h"

#define ERROR_BUFFER_SIZE (1024)

void jcunit_fatal_error(const char * fmt, ...)
{
    va_list args;
    static char buffer[ERROR_BUFFER_SIZE];

    va_start(args, fmt);
    vsnprintf(buffer, ERROR_BUFFER_SIZE, fmt, args);
    va_end(args);

    fprintf(stderr, "%s\n", buffer);
    exit(1);
}
