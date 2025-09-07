#include <string.h>

#include "headers/source.h"
#include "headers/object-allocator.h"
#include "headers/bytes-allocator.h"
#include "headers/util.h"

struct source * make_source(const char * filename, bool without_copying)
{
    struct source * source = alloc_source();
    if (without_copying) {
        source->filename = filename;
    } else {
        source->filename = duplicate_cstring(filename);
    }
    slist_init(&source->list_entry);
    return source;
}
