#include <string.h>

#include "headers/source.h"
#include "headers/util.h"
#include "headers/allocator.h"

struct source * make_source(const char * filename, bool without_copying)
{
    main_pool_alloc(struct source, source)
    if (without_copying) {
        source->filename = filename;
    } else {
        source->filename = duplicate_cstring(filename);
    }
    slist_init(&source->list_entry);
    return source;
}
