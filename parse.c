/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2022 Denis Korchagin <denis.korchagin.1995@gmail.com>
 *
 * This file is part of JCUnit
 *
 * For the full license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation of version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <stdio.h>
#include <stdlib.h>


#include "headers/parse.h"
#include "headers/ast.h"
#include "headers/allocate.h"


static struct ast_test * parse_test(struct tokenizer_context * context);
static void parse_test_prolog(struct tokenizer_context * context, struct ast_test ** ast_test);
static void parse_test_epilog(struct tokenizer_context * context);
static void parse_requirement_list(struct tokenizer_context * context, struct ast_test ** ast_test);
static void parse_requirement(struct tokenizer_context * context, struct ast_requirement * requirement);
static void parse_directive(struct tokenizer_context * context, struct string ** directive, struct string ** argument);
static void release_token(struct token * token);


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
                release_token(token);
            }
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
    if (ast_test->name == NULL) {
        fprintf(stderr, "Expected name for test!\n");
        exit(1);
    }
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
        fprintf(stderr, "There is no any requirement provided for test!\n");
        exit(1);
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
        if (loop_control > MAX_REQUIREMENT_COUNT) {
            fprintf(stderr, "Exceed max requirement for test!\n");
            break;
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
    parse_directive(context, &requirement->name, &requirement->argument);
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
            fprintf(stderr, "Unterminated test!\n");
            exit(1);
        }
        if (is_token_directive(token)) {
            unget_one_token(context, token);
            break;
        }
        last_ch = token->content.ch;
        *w++ = (char)last_ch;
        ++len;
        release_token(token);
    }
    if (!is_token_directive(token)) {
        fprintf(stderr, "The requirement's content of the test too long!\n");
        exit(1);
    }
    if (last_ch == '\n') {
        --len; /* we don't need to take a very last char of the requirement's content */
    }
    context->mode = oldmode;
    requirement->content = make_string(buffer, len);
}

void parse_test_prolog(struct tokenizer_context * context, struct ast_test ** ast_test)
{
    struct string * directive = NULL, * argument = NULL;
    parse_directive(context, &directive, &argument);
    if (!string_equals_with_cstring(directive, "test")) {
        fprintf(stderr, "Expected test directive, but given '%.*s'!\n", directive->len, directive->value);
        exit(1);
    }
    if (argument == NULL) {
        fprintf(stderr, "For test directive is require to specify a test name!\n");
        exit(1);
    }
    if (argument->len == 0) {
        fprintf(stderr, "The test must have a non-empty name!\n");
        exit(1);
    }
    (*ast_test)->name = argument;
}

void parse_test_epilog(struct tokenizer_context * context)
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
    release_token(token);
    token = get_one_token(context);
    if (!is_token_string(token)) {
        fprintf(stderr, "Expected string as directive argument!\n");
        exit(1);
    }
    (*argument) = token->content.string;
    release_token(token);
    token = get_one_token(context);
    if (!is_token_punctuator(token, ')')) {
        fprintf(stderr, "Expected ')' at end of directive argument!\n");
        exit(1);
    }
    release_token(token);
}

void parse_directive(struct tokenizer_context * context, struct string ** directive, struct string ** argument)
{
    struct token * token = get_one_token(context);

    if (!is_token_directive(token)) {
        fprintf(stderr, "Expected directive!\n");
        exit(1);
    }

    (*directive) = token->content.string;

    release_token(token);

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

    release_token(token);

    context->mode = oldmode;
}

void release_token(struct token * token)
{
    if (token == &newline_token || token == &eof_token) {
        return;
    }
    free_token(token);
}
