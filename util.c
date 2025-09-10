#include <stddef.h>
#include <string.h>


#include "headers/util.h"
#include "headers/allocator.h"


size_t align_up(const size_t size, const size_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

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
    const unsigned int len = strlen(src);
    char * dest = memory_blob_pool_alloc(&temporary_pool, len + 1);
    strcpy(dest, src);
    dest[len] = '\0';
    return dest;
}
