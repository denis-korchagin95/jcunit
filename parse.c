#include <string.h>


#include "headers/parse.h"
#include "headers/ast.h"
#include "headers/errors.h"
#include "headers/allocator.h"


struct directive_parse_context
{
    struct string ** directive;
    struct slist ** arguments_end;
    bool has_unnamed_argument;
};

static struct ast_test * parse_test(struct tokenizer_context * context);
static void parse_test_prolog(struct tokenizer_context * context, struct ast_test ** ast_test);
static void parse_test_epilog(struct tokenizer_context * context);
static void parse_requirement_list(struct tokenizer_context * context, struct ast_test ** ast_test);
static void parse_requirement(struct tokenizer_context * context, struct ast_requirement * requirement);
static void parse_directive(struct tokenizer_context * context, struct directive_parse_context * directive_parse_context);
static void parse_directive_argument(struct tokenizer_context * context, struct directive_parse_context * directive_parse_context);
static void parse_named_directive_argument(struct tokenizer_context * context, struct directive_parse_context * directive_parse_context);
static void skip_whitespaces(struct tokenizer_context * context);


struct slist * parse_test_suite(struct tokenizer_context * context)
{
    struct ast_test * ast_test;

    struct slist * tests = make_slist();
    struct slist ** tests_end = &tests->next;

    struct token * token = peek_one_token(context);

    while (!is_token_eof(token)) {
        if (is_token_newline(token)) { /* skip some newline between a tests */
            for(;;) {
                token = get_one_token(context);
                if (!is_token_newline(token)) {
                    unget_one_token(context, token);
                    break;
                }
            }
            continue;
        }
        ast_test = parse_test(context);

        slist_append(tests_end, &ast_test->list_entry);

        token = peek_one_token(context);
    }

    return tests;
}

struct ast_test * parse_test(struct tokenizer_context * context)
{
    struct ast_test * ast_test = make_ast_test();
    parse_test_prolog(context, &ast_test);
    context->mode = TOKENIZER_MODE_DIRECTIVE_AND_TEXT;
    parse_requirement_list(context, &ast_test);
    context->mode = TOKENIZER_MODE_NONE;
    parse_test_epilog(context);
    return ast_test;
}

void parse_requirement_list(struct tokenizer_context * context, struct ast_test ** ast_test)
{
    struct token * token = get_one_token(context);
    if (is_token_directive_equals(token, "endtest")) {
        jcunit_fatal_error("There is no any requirement provided for test!");
    }
    unget_one_token(context, token);

    unsigned int loop_control = 0;

    struct ast_requirement * requirement = NULL;

    struct slist ** requirements_end = slist_get_end(&(*ast_test)->requirements);

    for(;;) {
        token = peek_one_token(context);
        if (is_token_directive_equals(token, "endtest")) {
            break;
        }
        if (loop_control >= MAX_REQUIREMENT_COUNT) {
            jcunit_fatal_error("Exceed max requirement for test!");
        }
        ++loop_control;

        requirement = make_ast_requirement();
        parse_requirement(context, requirement);
        slist_append(requirements_end, &requirement->list_entry);
    }
}

void parse_requirement(struct tokenizer_context * context, struct ast_requirement * requirement)
{
    static char buffer[MAX_REQUIREMENT_CONTENT_SIZE] = {0};
    struct directive_parse_context directive_parse_context;
    directive_parse_context.directive = &requirement->name;
    directive_parse_context.arguments_end = &requirement->arguments.next;
    directive_parse_context.has_unnamed_argument = false;
    parse_directive(context, &directive_parse_context);
    struct token * token = peek_one_token(context);
    if (is_token_directive(token)) {
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
            jcunit_fatal_error("An unterminated test!");
        }
        if (is_token_directive(token)) {
            unget_one_token(context, token);
            break;
        }
        last_ch = token->content.ch;
        *w++ = (char)last_ch;
        ++len;
    }
    if (!is_token_directive(token)) { /* TODO: test it, make configurable 'max_requirement_content_size' */
        jcunit_fatal_error("The requirement's content of the test too long!");
    }
    if (last_ch == '\n') {
        --len; /* we don't need to take a very last char of the requirement's content */
    }
    context->mode = oldmode;
    requirement->content = make_string(buffer, len);
}

void parse_test_prolog(struct tokenizer_context * context, struct ast_test ** ast_test)
{
    struct string * directive = NULL;
    struct directive_parse_context directive_parse_context;
    directive_parse_context.directive = &directive;
    directive_parse_context.arguments_end = &(*ast_test)->arguments.next;
    directive_parse_context.has_unnamed_argument = false;
    parse_directive(context, &directive_parse_context);
    if (!string_equals_with_cstring(directive, "test")) {
        jcunit_fatal_error("Expected test directive, but given '%.*s'!", directive->len, directive->value);
    }
    directive = NULL;
}

void parse_test_epilog(struct tokenizer_context * context)
{
    struct string * directive = NULL;
    struct slist arguments;
    slist_init(&arguments);
    struct directive_parse_context directive_parse_context;
    directive_parse_context.directive = &directive;
    directive_parse_context.arguments_end = &arguments.next;
    directive_parse_context.has_unnamed_argument = false;
    parse_directive(context, &directive_parse_context);
    if (!string_equals_with_cstring(directive, "endtest")) { /* unreachable error for now */
        jcunit_fatal_error("Expected endtest directive, but given '%.*s'!", directive->len, directive->value);
    }
    if (!list_is_empty(&arguments)) {
        jcunit_fatal_error("Unexpected arguments for the 'endtest' directive!");
    }
    directive = NULL;
}

void parse_directive_arguments(struct tokenizer_context * context, struct directive_parse_context * directive_parse_context)
{
    struct token * token;
    for (;;) {
        skip_whitespaces(context);
        token = peek_one_token(context);
        if (is_token_punctuator(token, ')')) {
            break;
        }
        parse_directive_argument(context, directive_parse_context);
        token = get_one_token(context);
        if (!is_token_punctuator(token, ',')) {
            unget_one_token(context, token);
            break;
        }
    }
    token = get_one_token(context);
    if (!is_token_punctuator(token, ')')) {
        jcunit_fatal_error("Expected ')' at end of directive argument!");
    }
}

void parse_directive_argument(struct tokenizer_context * context, struct directive_parse_context * directive_parse_context)
{
    struct token * token = get_one_token(context);
    if (is_token_name(token)) {
        unget_one_token(context, token);
        return parse_named_directive_argument(context, directive_parse_context);
    }
    if (!is_token_string(token)) {
        jcunit_fatal_error("An unnamed argument of a directive should be a string!");
    }
    if (directive_parse_context->has_unnamed_argument) {
        jcunit_fatal_error("A directive cannot have two unnamed arguments!");
    }
    main_pool_alloc(struct ast_requirement_argument, argument)
    argument->name = NULL;
    argument->value = token->content.string;
    slist_append(directive_parse_context->arguments_end, &argument->list_entry);
    directive_parse_context->has_unnamed_argument = true;
}

void parse_named_directive_argument(struct tokenizer_context * context, struct directive_parse_context * directive_parse_context)
{
    struct token * token = get_one_token(context);
    struct string * name = NULL;
    name = token->content.string;
    token = get_one_token(context);
    if (!is_token_punctuator(token, '=')) {
        jcunit_fatal_error("Expected '=' after name of argument!");
    }
    token = get_one_token(context);
    if (!is_token_string(token)) {
        jcunit_fatal_error("Expected value of the named argument as a string!");
    }
    main_pool_alloc(struct ast_requirement_argument, argument)
    argument->name = name;
    argument->value = token->content.string;
    slist_append(directive_parse_context->arguments_end, &argument->list_entry);
}

void parse_directive(struct tokenizer_context * context, struct directive_parse_context * directive_parse_context)
{
    struct token * token = get_one_token(context);

    if (!is_token_directive(token)) {
        jcunit_fatal_error("Expected a directive!");
    }

    *directive_parse_context->directive = token->content.string;

    unsigned int oldmode = context->mode;
    context->mode = TOKENIZER_MODE_NONE;

    int ch = peek_one_char(context);

    if (ch == '(') {
        token = get_one_token(context);

        parse_directive_arguments(context, directive_parse_context);
    }

    token = get_one_token(context);

    if (!is_token_newline(token)) {
        jcunit_fatal_error("Expected newline character after directive!");
    }

    context->mode = oldmode;
}

void skip_whitespaces(struct tokenizer_context * context)
{
    int ch = get_one_char(context);
    while (is_whitespace_char(ch)) {
        ch = get_one_char(context);
    }
    unget_one_char(context, ch);
}
