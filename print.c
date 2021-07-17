/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2021 Denis Korchagin <denis.korchagin.1995@gmail.com>
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

void print_ast_test_case(struct ast_test_case * test_case, FILE * output)
{
    fprintf(output, "test: %.*s\n", test_case->name->len, test_case->name->value);
    slist_foreach(iterator, &test_case->requirements, {
        struct ast_requirement * requirement = list_get_owner(iterator, struct ast_requirement, list_entry);
        print_ast_requirement(requirement, output);
    });
}

void print_ast_requirement(struct ast_requirement * requirement, FILE * output)
{
    fprintf(output, "Requirement\n");
    fprintf(output, "\tName: %.*s\n", requirement->name->len, requirement->name->value);
    if (requirement->argument != NULL) {
        fprintf(output, "\tArgument: %.*s\n", requirement->argument->len, requirement->argument->value);
    } else {
        fprintf(output, "\tArgument: <not provided>\n");
    }
    if (requirement->content != NULL) {
        fprintf(output, "\tContent: %.*s\n", requirement->content->len, requirement->content->value);
    } else {
        fprintf(output, "\tContent: <not provided>\n");
    }
}
