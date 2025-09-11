#ifndef JCUNIT_UTIL_H
#define JCUNIT_UTIL_H 1

#include <stddef.h>

size_t align_up(size_t size, size_t alignment);

const char * duplicate_cstring(const char * path);
const char * basename(const char * path);

void cleanup(void);

#endif /* JCUNIT_UTIL_H */
