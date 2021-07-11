#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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
    int i;
    char * arg;
    char * filename = NULL;
    for(i = 1; i < argc; ++i) {
        arg = argv[i];
        if (strncmp("--show-allocator-stats", arg, sizeof("--show-allocator-stats")) == 0) {
            option_show_allocator_stats = true;
            continue;
        }
        if (strncmp("--", arg, 2) == 0) {
            fprintf(stderr, "The unknown option: %s\n", arg);
            exit(1);
        }
        filename = arg;
    }

    if (filename == NULL) {
        fprintf(stderr, "The filename is missing!\n");
        exit(1);
    }

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(filename);
    struct list * ast_cases = parse_test(context);
    destroy_tokenizer_context(context);

    struct test * test = assemble_test(filename, ast_cases);

    struct test_runner_context * runner_context = make_test_runner_context(test);

    test_run(runner_context);

    show_test_result(runner_context, stdout);

    if (option_show_allocator_stats) {
        printf("\n\n\n");

        show_allocator_stats(stdout, SHOW_STAT_ALL_ALLOCATORS);
    }

    return 0;
}
