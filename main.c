#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "headers/ast.h"
#include "headers/token.h"
#include "headers/list.h"
#include "headers/allocate.h"
#include "headers/string.h"
#include "headers/print.h"


struct test * parse_test(struct tokenizer_context * context);
struct test_case * parse_test_case(struct tokenizer_context * context);

void parse_test_case_prolog(struct tokenizer_context * context, struct test_case ** test_case);
void parse_test_case_epilog(struct tokenizer_context * context, struct test_case ** test_case);
void parse_directive_parameter(struct tokenizer_context * context, struct string ** parameter);

bool is_token_directive(struct token * token, const char * directive_name);

struct test * make_test(void);
struct test_case * make_test_case(void);

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

    struct test * test = parse_test(context);

    fclose(source);

    return 0;
}

struct test * parse_test(struct tokenizer_context * context)
{
    struct test_case * test_case = parse_test_case(context);

    struct test * test = make_test();
    list_add(&test->cases, &test_case->list_entry);
    return test;
}

struct test_case * parse_test_case(struct tokenizer_context * context)
{
    struct test_case * test_case = make_test_case();
    parse_test_case_prolog(context, &test_case);
    if (test_case->name == NULL) {
        fprintf(stderr, "Expected name for test case!\n");
        exit(1);
    }
    parse_test_case_epilog(context, &test_case);
    return test_case;
}

void parse_test_case_prolog(struct tokenizer_context * context, struct test_case ** test_case)
{
    struct token * token = get_one_token(context);
    if (!is_token_directive(token, "test")) {
        fprintf(stderr, "Expected test directive!\n");
        exit(1);
    }
    token = get_one_token(context);
    struct string * directive_parameter = NULL;
    if (is_token_punctuator(token, '(')) {
        parse_directive_parameter(context, &directive_parameter);
        token = get_one_token(context);
        if (!is_token_punctuator(token, ')')) {
            fprintf(stderr, "Expected end of directive parameter!\n");
            exit(1);
        }
        token = get_one_token(context);
        if (!is_token_newline(token)) {
            fprintf(stderr, "Expected newline character after directive!\n");
            exit(1);
        }
    } else {
        unget_one_token(context, token);
    }
    (*test_case)->name = directive_parameter;
}

void parse_test_case_epilog(struct tokenizer_context * context, struct test_case ** test_case)
{
    struct token * token = get_one_token(context);
    if (!is_token_directive(token, "endtest")) {
        fprintf(stderr, "Expected endtest directive!\n");
        exit(1);
    }
    token = get_one_token(context);
    if (!is_token_newline(token)) {
        fprintf(stderr, "Expected newline character after directive!\n");
        exit(1);
    }
}

void parse_directive_parameter(struct tokenizer_context * context, struct string ** parameter)
{
    struct token * token = get_one_token(context);
    if (!is_token_string(token)) {
        fprintf(stderr, "Expected parameter as string for test case!\n");
        exit(1);
    }
    (*parameter) = token->content.string;
}

struct test * make_test(void)
{
    struct test * test = alloc_test();
    list_init(&test->cases);
    return test;
}

struct test_case * make_test_case(void)
{
    struct test_case * test_case = alloc_test_case();
    list_init(&test_case->requirements);
    memset((void *)&test_case->list_entry, 0, sizeof(struct list));
    test_case->name = NULL;
    return test_case;
}
