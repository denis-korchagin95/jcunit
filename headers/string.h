#ifndef JCUNIT_STRING_H
#define JCUNIT_STRING_H

#include <stdbool.h>

struct string
{
    char * value;
    unsigned int len;
};

struct string * make_string(const char * source, unsigned int len);

bool string_equals_with_cstring(struct string * source, const char * dest);

#endif /* JCUNIT_STRING_H */
