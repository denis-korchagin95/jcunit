#include <stdio.h>

#include "../headers/token.h"
#include "../headers/print.h"
#include "../headers/parse.h"
#include "../headers/errors.h"


int main(int argc, char * argv[])
{
    if (argc <= 1) {
        jcunit_fatal_error("No specified args!");
    }

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(argv[1]);
    struct slist * list = parse_test_suite(context);
    destroy_tokenizer_context(context);

    struct ast_test * test = list_get_owner(list->next, struct ast_test, list_entry);

    print_ast_test(test, stdout);

    return 0;
}
