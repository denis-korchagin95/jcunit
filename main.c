#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "headers/token.h"
#include "headers/list.h"
#include "headers/parse.h"
#include "headers/assembler.h"
#include "headers/runner.h"
#include "headers/show_result.h"
#include "headers/allocate.h"

bool option_show_allocator_stats = false;

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

    int i;
    char * arg;
    for(i = 2; i < argc; ++i) {
        arg = argv[i];
        if (strncmp("--show-allocator-stats", arg, sizeof("--show-allocator-stats")) == 0) {
            option_show_allocator_stats = true;
            continue;
        }
    }

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(source);
    struct list * ast_cases = parse_test(context);

    fclose(source);

    struct test * test = assemble_test(ast_cases);

    struct test_runner_context * runner_context = make_test_runner_context();

    test_run(runner_context, test);

    show_test_result(runner_context, stdout);

    if (option_show_allocator_stats) {
        printf("\n\n\n");

        show_allocator_stats(stdout, SHOW_STAT_ALL_ALLOCATORS);
    }

    return 0;
}
