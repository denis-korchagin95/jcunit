#include <memory.h>


#include "headers/string.h"
#include "headers/allocate.h"


struct string * make_string(const char * source, unsigned int len)
{
    char * storage = alloc_bytes(len);
    memcpy((void *)storage, source, len * sizeof(char));

    struct string * string = alloc_string();
    string->len = len;
    string->value = storage;

    return string;
}
