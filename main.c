#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "headers/ast.h"
#include "headers/token.h"
#include "headers/list.h"
#include "headers/print.h"

struct list * parse_test(struct tokenizer_context * context);

int main(int argc, char * argv[])
{
    if (argc <= 1) {
        fprintf(stderr, "No specified args!\n");
        return 1;
    }
    FILE * source = fopen(argv[1], "r");
    if (source == NULL) {
        fprintf(stderr, "Can't read file: %s\n", strerror(errno));
        return 1;
    }

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(source);

    struct list * cases = parse_test(context);

    struct list * iterator = cases->next;
    while (iterator != cases) {
        struct ast_test_case * test_case = list_get_owner(iterator, struct ast_test_case, list_entry);
        print_ast_test_case(test_case, stdout);
        puts("");
        iterator = iterator->next;
    }

    fclose(source);

    return 0;
}
