#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "headers/token.h"
#include "headers/list.h"
#include "headers/parse.h"
#include "headers/assembler.h"

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
    struct list * ast_cases = parse_test(context);
    struct test * test = assemble_test(ast_cases);

    fclose(source);

    return 0;
}
