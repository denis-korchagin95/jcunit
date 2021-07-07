#include <assert.h>


#include "headers/print.h"
#include "headers/string.h"


void print_token(struct token * token, FILE * output)
{
    assert(token != NULL);
    switch (token->kind) {
        case TOKEN_KIND_CHARACTER:
            fprintf(output, "<TOKEN_CHARACTER '%c'>", token->content.ch);
            break;
        case TOKEN_KIND_PUNCTUATOR:
            fprintf(output, "<TOKEN_PUNCTUATOR '%c'>", token->content.ch);
            break;
        case TOKEN_KIND_DIRECTIVE:
            fprintf(output, "<TOKEN_DIRECTIVE '%.*s'>", token->content.string->len, token->content.string->value);
            break;
        case TOKEN_KIND_STRING:
            fprintf(output, "<TOKEN_STRING '%.*s'>", token->content.string->len, token->content.string->value);
            break;
        case TOKEN_KIND_NEWLINE:
            fprintf(output, "<TOKEN_NEWLINE '\\n'>");
            break;
        case TOKEN_KIND_EOF:
            fprintf(output, "<TOKEN_EOF>");
            break;
        default:
            fprintf(output, "<unknown token>");
    }
}
