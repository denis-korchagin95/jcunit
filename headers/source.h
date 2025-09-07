#ifndef JCUNIT_SOURCE_H
#define JCUNIT_SOURCE_H 1

#include <stdbool.h>

#include "list.h"

struct source {
    struct slist list_entry;
    const char * filename;
};

struct source * make_source(const char * filename, bool without_copying);

#endif /* JCUNIT_SOURCE_H */
