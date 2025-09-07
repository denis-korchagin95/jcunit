#include "../headers/token.h"
#include "../headers/print.h"
#include "../headers/compiler.h"
#include "../headers/errors.h"
#include "../headers/application.h"
#include "../headers/source.h"


int main(int argc, char * argv[])
{
    if (argc <= 1) {
        jcunit_fatal_error("No specified args!");
    }

    struct slist sources;

    fetch_sources(argc, argv, &sources);

    init_tokenizer();
    (void)compile_test_suite(list_get_owner(sources.next, struct source, list_entry));

    return 0;
}
