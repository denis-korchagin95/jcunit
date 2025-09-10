#include <stdio.h>

#include "../headers/allocator.h"
#include "../headers/token.h"
#include "../headers/print.h"
#include "../headers/errors.h"


int main(int argc, char * argv[])
{
    if (argc <= 1) {
        jcunit_fatal_error("No specified args!");
    }

    memory_blob_pool_init_pools();

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(argv[1]);

    struct token * token;
    for(;;) {
        token = get_one_token(context);
        print_token(token, stdout);
        puts("");
        if (token->kind == TOKEN_KIND_EOF) {
            break;
        }
    }

    memory_blob_pool_destroy_pools();

    return 0;
}
