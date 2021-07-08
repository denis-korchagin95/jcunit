#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "headers/ast.h"
#include "headers/token.h"
#include "headers/list.h"
#include "headers/string.h"


struct list * parse_test(struct tokenizer_context * context);

static struct ast_test_case * parse_test_case(struct tokenizer_context * context);
static void parse_test_case_prolog(struct tokenizer_context * context, struct ast_test_case ** ast_test_case);
static void parse_test_case_epilog(struct tokenizer_context * context);
static void parse_requirement_list(struct tokenizer_context * context, struct ast_test_case ** ast_test_case);
static void parse_directive(struct tokenizer_context * context, struct string ** directive, struct string ** argument);

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

    fclose(source);

    return 0;
}

struct list * parse_test(struct tokenizer_context * context)
{
    struct ast_test_case * ast_test_case;

    struct list * cases = make_list();

    struct token * token = peek_one_token(context);

    while (!is_token_eof(token)) {
        ast_test_case = parse_test_case(context);

        list_add(cases, &ast_test_case->list_entry);

        token = peek_one_token(context);
    }

    return cases;
}

struct ast_test_case * parse_test_case(struct tokenizer_context * context)
{
    struct ast_test_case * ast_test_case = make_ast_test_case();
    parse_test_case_prolog(context, &ast_test_case);
    if (ast_test_case->name == NULL) {
        fprintf(stderr, "Expected name for test case!\n");
        exit(1);
    }
    context->mode = TOKENIZER_MODE_DIRECTIVE_AND_TEXT;
    parse_requirement_list(context, &ast_test_case);
    context->mode = TOKENIZER_MODE_NONE;
    parse_test_case_epilog(context);
    return ast_test_case;
}

void parse_requirement_list(struct tokenizer_context * context, struct ast_test_case ** ast_test_case)
{
}

void parse_test_case_prolog(struct tokenizer_context * context, struct ast_test_case ** ast_test_case)
{
    struct string * directive = NULL, * argument = NULL;
    parse_directive(context, &directive, &argument);
    if (!string_equals_with_cstring(directive, "test")) {
        fprintf(stderr, "Expected test directive, but given '%.*s'!\n", directive->len, directive->value);
        exit(1);
    }
    if (argument == NULL) {
        fprintf(stderr, "For test directive is require to specify a test case name!\n");
        exit(1);
    }
    (*ast_test_case)->name = argument;
}

void parse_test_case_epilog(struct tokenizer_context * context)
{
    struct string * directive = NULL, * argument = NULL;
    parse_directive(context, &directive, &argument);
    if (!string_equals_with_cstring(directive, "endtest")) {
        fprintf(stderr, "Expected endtest directive, but given '%.*s'!\n", directive->len, directive->value);
        exit(1);
    }
}

void parse_directive_argument(struct tokenizer_context * context, struct string ** argument)
{
    struct token * token = get_one_token(context);
    if (!is_token_punctuator(token, '(')) {
        fprintf(stderr, "Expected '(' at begin of directive argument!\n");
        exit(1);
    }
    token = get_one_token(context);
    if (!is_token_string(token)) {
        fprintf(stderr, "Expected string as directive argument!\n");
        exit(1);
    }
    (*argument) = token->content.string;
    token = get_one_token(context);
    if (!is_token_punctuator(token, ')')) {
        fprintf(stderr, "Expected ')' at end of directive argument!\n");
        exit(1);
    }
}

void parse_directive(struct tokenizer_context * context, struct string ** directive, struct string ** argument)
{
    struct token * token = get_one_token(context);

    if (token->kind != TOKEN_KIND_DIRECTIVE) {
        fprintf(stderr, "Expected directive!\n");
        exit(1);
    }

    (*directive) = token->content.string;

    unsigned int oldmode = context->mode;
    context->mode = TOKENIZER_MODE_NONE;

    int ch = peek_one_char(context);

    if (ch == '(') {
        parse_directive_argument(context, argument);
    }

    token = get_one_token(context);

    if (!is_token_newline(token)) {
        fprintf(stderr, "Expected newline character after directive!\n");
        exit(1);
    }

    context->mode = oldmode;
}
