#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "headers/token.h"
#include "headers/list.h"
#include "headers/parse.h"
#include "headers/assembler.h"
#include "headers/runner.h"
#include "headers/show_result.h"


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

    struct test_runner_context * runner_context = make_test_runner_context();

    test_run(runner_context, test);

    show_test_result(runner_context, stdout);

    fclose(source);

    return 0;
}
