#include <stddef.h>


#include "headers/util.h"


const char * basename(const char * path)
{
    const char * iterator = path;
    const char * last_slash_pos = NULL;
    while (*iterator != '\0') {
        if (*iterator == '/') {
            last_slash_pos = iterator;
        }
        ++iterator;
    }
    if (last_slash_pos == NULL) {
        if (*path == '\0') {
            return NULL;
        }
        return path;
    }
    return last_slash_pos + 1;
}
