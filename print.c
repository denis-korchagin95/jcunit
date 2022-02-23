/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2021-2022 Denis Korchagin <denis.korchagin.1995@gmail.com>
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
#include <assert.h>


#include "headers/print.h"

#define INDENT_WIDTH (4)

static void do_print_ast_requirement_arguments(struct ast_requirement * requirement, FILE * output, unsigned int depth);
static void do_print_ast_requirements(struct ast_test * test, FILE * output, unsigned int depth);
static void do_print_ast_requirement(struct ast_requirement * requirement, FILE * output, unsigned int depth, unsigned int requirement_number);
static void do_print_ast_test(struct ast_test * test, FILE * output, unsigned int depth);
static void do_indent(FILE * output, unsigned int depth);


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
        case TOKEN_KIND_NAME:
            fprintf(output, "<TOKEN_NAME '%.*s'>", token->content.string->len, token->content.string->value);
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

void print_ast_test(struct ast_test * test, FILE * output)
{
    do_print_ast_test(test, output, 0);
}

void do_print_ast_test(struct ast_test * test, FILE * output, unsigned int depth)
{
    fprintf(output, "Test:\n");
    do_indent(output, depth + 1);
    fprintf(output, "Arguments:\n");
    do_indent(output, depth + 2);
    fprintf(output, "Argument #1: %s=\"%s\"\n", "name", test->name->value);
    do_indent(output, depth + 1);
    do_print_ast_requirements(test, output, depth + 1);
}

void do_print_ast_requirements(struct ast_test * test, FILE * output, unsigned int depth)
{
    fprintf(output, "Requirements:\n");
    unsigned int requirement_number = 0;
    slist_foreach(iterator, &test->requirements, {
        struct ast_requirement * requirement = list_get_owner(iterator, struct ast_requirement, list_entry);
        ++requirement_number;
        do_indent(output, depth + 1);
        do_print_ast_requirement(requirement, output, depth + 1, requirement_number);
    });
}

void print_ast_requirement(struct ast_requirement * requirement, FILE * output)
{
    do_print_ast_requirement(requirement, output, 0, 1);
}

void do_print_ast_requirement(struct ast_requirement * requirement, FILE * output, unsigned int depth, unsigned int requirement_number)
{
    fprintf(output, "Requirement #%u:\n", requirement_number);
    do_indent(output, depth + 1);
    fprintf(output, "Name: \"%s\"\n", requirement->name->value);
    do_indent(output, depth + 1);
    do_print_ast_requirement_arguments(requirement, output, depth + 1);
    do_indent(output, depth + 1);
    if (requirement->content == NULL) {
        fprintf(output, "Content: <not provided>\n");
    } else {
        fprintf(output, "Content: \"%s\"\n", requirement->content->value);
    }
}

void do_print_ast_requirement_arguments(struct ast_requirement * requirement, FILE * output, unsigned int depth)
{
    fprintf(output, "Arguments:\n");
    unsigned int argument_number = 0;
    slist_foreach(iterator, &requirement->arguments, {
        struct ast_requirement_argument * argument = list_get_owner(iterator, struct ast_requirement_argument, list_entry);
        ++argument_number;
        do_indent(output, depth + 1);
        if (argument->name != NULL) {
            fprintf(output, "Argument #%u: %s=\"%s\"\n", argument_number, argument->name->value, argument->value->value);
        } else {
            fprintf(output, "Argument #%u: \"%s\" (unnamed)\n", argument_number, argument->value->value);
        }
    });
}

static void do_indent(FILE * output, unsigned int depth)
{
    unsigned int i, len;
    for (i = 0, len = depth * INDENT_WIDTH; i < len; ++i) {
        fprintf(output, " ");
    }
}
