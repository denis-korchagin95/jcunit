#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../headers/token.h"
#include "../headers/print.h"
#include "../headers/errors.h"


int main(int argc, char * argv[])
{
    if (argc <= 1) {
        jcunit_fatal_error("No specified args!");
    }
    int token_number = -1;
    int i;
    for (i = 1; i < argc; ++i) {
        char * arg = argv[i];
        if (strncmp("--turn-special-mode-after-token=", arg, sizeof("--turn-special-mode-after-token=") - 1) == 0) {
            arg += (sizeof("--turn-special-mode-after-token=") - 1);
            if (!isdigit(*arg)) {
                jcunit_fatal_error("Bad token number of option \"--turn-special-mode-after-token\"!");
            }
            token_number = atoi(arg);
        }
    }

    init_tokenizer();

    struct tokenizer_context * context = make_tokenizer_context(argv[1]);

    int token_counter = 0;

    struct token * token;
    for(;;) {
        token = get_one_token(context);
        ++token_counter;
        if (token_number != -1 && token_counter > token_number) {
            context->mode = TOKENIZER_MODE_DIRECTIVE_AND_TEXT;
        }
        print_token(token, stdout);
        puts("");
        if (token->kind == TOKEN_KIND_EOF) {
            break;
        }
    }

    destroy_tokenizer_context(context);

    return 0;
}
