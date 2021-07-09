#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "headers/ast.h"
#include "headers/token.h"
#include "headers/list.h"
#include "headers/string.h"
#include "headers/print.h"

struct list * parse_test(struct tokenizer_context * context);

static struct ast_test_case * parse_test_case(struct tokenizer_context * context);
static void parse_test_case_prolog(struct tokenizer_context * context, struct ast_test_case ** ast_test_case);
static void parse_test_case_epilog(struct tokenizer_context * context);
static void parse_requirement_list(struct tokenizer_context * context, struct ast_test_case ** ast_test_case);
static void parse_requirement(struct tokenizer_context * context, struct ast_requirement * requirement);
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

    struct list * iterator = cases->next;
    while (iterator != cases) {
        struct ast_test_case * test_case = list_get_owner(iterator, struct ast_test_case, list_entry);
        print_ast_test_case(test_case, stdout);
        iterator = iterator->next;
    }

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
    struct token * token = get_one_token(context);
    if (is_token_directive(token, "endtest")) {
        fprintf(stderr, "There is no any requirement provided for test case!\n");
        exit(1);
    }
    unget_one_token(context, token);

    unsigned int loop_control = 0;

    struct ast_requirement * requirement = NULL;

    for(;;) {
        token = peek_one_token(context);
        if (is_token_directive(token, "endtest")) {
            break;
        }
        if (loop_control > MAX_REQUIREMENT_COUNT) {
            fprintf(stderr, "Exceed max requirement for test case!\n");
            break;
        }
        ++loop_control;

        requirement = make_ast_requirement();
        parse_requirement(context, requirement);
        list_add(&(*ast_test_case)->requirements, &requirement->list_entry);
    }
}

void parse_requirement(struct tokenizer_context * context, struct ast_requirement * requirement)
{
    static char buffer[MAX_REQUIREMENT_CONTENT_SIZE] = {0};
    parse_directive(context, &requirement->name, &requirement->argument);
    struct token * token = peek_one_token(context);
    if (token->kind == TOKEN_KIND_DIRECTIVE) {
        return; /* no content for requirement */
    }
    unsigned int oldmode = context->mode;
    context->mode = TOKENIZER_MODE_DIRECTIVE_AND_TEXT;
    unsigned int len = 0;
    char * w = buffer;
    int last_ch = token->content.ch;
    while (len < MAX_REQUIREMENT_CONTENT_SIZE) {
        token = get_one_token(context);
        if (is_token_eof(token)) {
            fprintf(stderr, "Unterminated test case!\n");
            exit(1);
        }
        if (token->kind == TOKEN_KIND_DIRECTIVE) {
            unget_one_token(context, token);
            break;
        }
        last_ch = token->content.ch;
        *w++ = (char)last_ch;
        ++len;
    }
    if (token->kind != TOKEN_KIND_DIRECTIVE) {
        fprintf(stderr, "The requirement's content of the test case too long!\n");
        exit(1);
    }
    if (last_ch == '\n') {
        --len; /* we don't need to take a very last char of the requirement's content */
    }
    context->mode = oldmode;
    requirement->content = make_string(buffer, len);
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
