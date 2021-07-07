#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "headers/token.h"
#include "headers/print.h"


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

    struct token * token;
    for(;;) {
        token = get_one_token(context);
        print_token(token, stdout);
        puts("");
        if (token->kind == TOKEN_KIND_EOF) {
            break;
        }
    }

    fclose(source);

    return 0;
}
