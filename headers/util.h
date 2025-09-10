#ifndef JCUNIT_UTIL_H
#define JCUNIT_UTIL_H 1

#include <stddef.h>

size_t align_up(const size_t size, const size_t alignment);

const char * duplicate_cstring(const char * path);
const char * basename(const char * path);

#endif /* JCUNIT_UTIL_H */
