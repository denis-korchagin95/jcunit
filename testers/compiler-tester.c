#include <stdlib.h>

#include "../headers/token.h"
#include "../headers/compiler.h"
#include "../headers/errors.h"
#include "../headers/application.h"
#include "../headers/source.h"
#include "../headers/allocator.h"
#include "../headers/util.h"

int main(int argc, char * argv[])
{
    if (argc <= 1) {
        jcunit_fatal_error("No specified args!");
    }

    if (atexit(cleanup) != 0) {
        jcunit_fatal_error("Can't register atexit handler!");
    }

    memory_blob_pool_init_pools();

    struct slist sources;

    fetch_sources(argc, argv, &sources);

    init_tokenizer();
    (void)compile_test_suite(list_get_owner(sources.next, struct source, list_entry));

    return 0;
}
