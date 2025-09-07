#include <stddef.h>
#include <string.h>


#include "headers/util.h"
#include "headers/bytes-allocator.h"


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

const char * duplicate_cstring(const char * src)
{
    unsigned int len = strlen(src);
    char * dest = alloc_bytes(len + 1);
    strcpy(dest, src);
    dest[len] = '\0';
    return dest;
}
