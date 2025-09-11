#include <memory.h>


#include "headers/string.h"
#include "headers/allocator.h"


struct string * make_string(const char * source, unsigned int len)
{
    char * storage = memory_blob_pool_alloc(&temporary_pool, len + 1);
    memcpy((void *)storage, source, len * sizeof(char));
    storage[len] = '\0';

    main_pool_alloc(struct string, string)
    string->len = len;
    string->value = storage;
    string->flags = 0;

    return string;
}


bool string_equals_with_cstring(struct string * source, const char * dest)
{
    unsigned int len = strlen(dest);
    return source->len == len && strncmp(source->value, dest, len) == 0;
}
