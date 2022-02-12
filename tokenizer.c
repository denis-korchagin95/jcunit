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
#include <assert.h>
#include <string.h>
#include <errno.h>


#include "headers/token.h"
#include "headers/allocate.h"
#include "headers/string.h"


struct token newline_token = {0}, eof_token = {0};


struct token * make_punctuator_token(int ch)
{
    struct token * token = alloc_token();
    token->kind = TOKEN_KIND_PUNCTUATOR;
    token->content.ch = ch;
    return token;
}

struct token * get_one_string(struct tokenizer_context * context, int ch)
{
    static char buffer[MAX_STRING_LEN] = {0};
    char * w = buffer;
    ch = get_one_char(context);
    unsigned int len = 0;
    while (len < MAX_STRING_LEN && ch != '"') {
        if (ch == EOF) {
            fprintf(stderr, "Unterminated string!\n");
            exit(1);
        }
        if (ch == '\n') {
            fprintf(stderr, "Newline inside the string!\n");
            exit(1);
        }
        *w++ = (char)ch;
        ++len;
        ch = get_one_char(context);
    }
    if (ch != '"') {
        fprintf(stderr, "Too long string!\n");
        exit(1);
    }
    struct token * token = alloc_token();
    token->kind = TOKEN_KIND_STRING;
    token->content.string = make_string(buffer, len);
    return token;
}

struct tokenizer_context * make_tokenizer_context(const char * filename)
{
    struct tokenizer_context * context = alloc_tokenizer_context();
    FILE * source = fopen(filename, "r");
    if (source == NULL) {
        fprintf(stderr, "Can't read file \"%s\": %s\n", filename, strerror(errno));
        exit(1);
    }
    memset((void *)context, 0, sizeof(struct tokenizer_context));
    context->filename = filename;
    context->source = source;
    return context;
}

void destroy_tokenizer_context(struct tokenizer_context * context)
{
    assert(context != NULL);

    if (context->source != NULL) {
        fclose(context->source);
    }
    free_tokenizer_context(context);
}

void init_tokenizer(void)
{
    struct token * token;

    token = &eof_token;
    token->kind = TOKEN_KIND_EOF;
    token->content.ch = EOF;

    token = &newline_token;
    token->kind = TOKEN_KIND_NEWLINE;
    token->content.ch = '\n';
}

int get_one_char(struct tokenizer_context * context)
{
    if (context->char_buffer_pos > 0)
        return context->char_buffer[--context->char_buffer_pos];
    return fgetc(context->source);
}

void unget_one_char(struct tokenizer_context * context, int ch)
{
    assert(context->char_buffer_pos < MAX_CHAR_BUFFER_SIZE);
    if (context->char_buffer_pos >= MAX_CHAR_BUFFER_SIZE) {
        fprintf(stderr, "The char buffer is overflow!\n");
        return;
    }
    if (ch != EOF)
        context->char_buffer[context->char_buffer_pos++] = (char)ch;
}

int peek_one_char(struct tokenizer_context * context)
{
    int ch = get_one_char(context);
    unget_one_char(context, ch);
    return ch;
}

struct token * get_one_directive(struct tokenizer_context * context, int ch)
{
    static char buffer[MAX_NAME_LEN] = {0};

    char * w = buffer;

    ch = get_one_char(context);

    while (is_name_char(ch)) {
        *w++ = (char)ch;
        ch = get_one_char(context);
    }
    unget_one_char(context, ch);

    struct token * token = alloc_token();
    token->kind = TOKEN_KIND_DIRECTIVE;
    token->content.string = make_string(buffer, w - buffer);
    return token;
}

struct token * get_one_token(struct tokenizer_context * context)
{
    if (context->token_buffer_pos > 0)
        return context->token_buffer[--context->token_buffer_pos];

    int ch = get_one_char(context);

    if (ch == EOF) {
        return &eof_token;
    }

    if (ch == '@') {
        int next = get_one_char(context);
        if (is_start_name_char(next)) {
            unget_one_char(context, next);
            return get_one_directive(context, ch);
        } else if (next == '@') {
            goto character;
        }
        unget_one_char(context, next);
        goto character;
    }

    if (context->mode == TOKENIZER_MODE_DIRECTIVE_AND_TEXT) {
        goto character;
    }

    if (ch == '\n') {
        return &newline_token;
    }

    if (ch == '(') {
        return make_punctuator_token(ch);
    }

    if (ch == ')') {
        return make_punctuator_token(ch);
    }

    if (ch == '"') {
        return get_one_string(context, ch);
    }

    struct token * token;

character:
    token = alloc_token();
    token->kind = TOKEN_KIND_CHARACTER;
    token->content.ch = ch;
    return token;
}

void unget_one_token(struct tokenizer_context * context, struct token * token)
{
    assert(token != NULL);
    if (context->token_buffer_pos >= MAX_TOKEN_BUFFER_SIZE) {
        fprintf(stderr, "The token buffer is overflow!\n");
        exit(1);
    }
    if (!is_token_eof(token))
        context->token_buffer[context->token_buffer_pos++] = token;
}

struct token * peek_one_token(struct tokenizer_context * context)
{
    struct token * token = get_one_token(context);
    unget_one_token(context, token);
    return token;
}

bool is_token_directive_equals(struct token * token, const char * name)
{
    return token->kind == TOKEN_KIND_DIRECTIVE && string_equals_with_cstring(token->content.string, name);
}

bool is_token_punctuator(struct token * token, int ch)
{
    if (token->kind != TOKEN_KIND_PUNCTUATOR)
        return false;
    return token->content.ch == ch;
}
