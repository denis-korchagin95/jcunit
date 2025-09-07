#include <memory.h>


#include "headers/string.h"
#include "headers/object-allocator.h"
#include "headers/bytes-allocator.h"

struct string * make_string(const char * source, unsigned int len)
{
    char * storage = alloc_bytes(len + 1);
    memcpy((void *)storage, source, len * sizeof(char));
    storage[len] = '\0';

    struct string * string = alloc_string();
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
