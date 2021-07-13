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
#ifndef JCUNIT_TOKEN_H
#define JCUNIT_TOKEN_H 1

#include <stdio.h>
#include <stdbool.h>

#define TOKEN_KIND_EOF           (-1)
#define TOKEN_KIND_DIRECTIVE     (1)
#define TOKEN_KIND_PUNCTUATOR    (2)
#define TOKEN_KIND_STRING        (3)
#define TOKEN_KIND_NEWLINE       (4)
#define TOKEN_KIND_CHARACTER     (5)

#define TOKENIZER_MODE_NONE                 (0)
#define TOKENIZER_MODE_DIRECTIVE_AND_TEXT   (1)

#define MAX_TOKEN_BUFFER_SIZE (4)
#define MAX_CHAR_BUFFER_SIZE (4)
#define MAX_NAME_LEN (256)
#define MAX_STRING_LEN (1024)
#define MAX_REQUIREMENT_CONTENT_SIZE (8192)
#define MAX_REQUIREMENT_COUNT (12)

#define is_upper_char(ch) ((ch) >= 'A' && (ch) <= 'Z')
#define is_lower_char(ch) ((ch) >= 'a' && (ch) <= 'z')
#define is_char(ch) (is_lower_char(ch) || is_upper_char(ch))
#define is_digit_char(ch) ((ch) >= '0' && (ch) <= '9')
#define is_start_name_char(ch) (is_char(ch) || (ch) == '_')
#define is_name_char(ch) (is_char(ch) || (ch) == '_' || is_digit_char(ch))

#define is_token_string(token) ((token)->kind == TOKEN_KIND_STRING)
#define is_token_directive(token) ((token)->kind == TOKEN_KIND_DIRECTIVE)
#define is_token_eof(token) ((token) == &eof_token)
#define is_token_newline(token) ((token) == &newline_token)

struct token
{
    unsigned int kind;
    union
    {
        int ch;
        struct string * string;
    } content;
};

struct tokenizer_context
{
    const char * filename;
    FILE * source;
    struct token * token_buffer[MAX_TOKEN_BUFFER_SIZE];
    unsigned int token_buffer_pos;
    char char_buffer[MAX_CHAR_BUFFER_SIZE];
    unsigned int char_buffer_pos;
    unsigned int mode;
};

extern struct token newline_token, eof_token;

void init_tokenizer(void);

struct tokenizer_context * make_tokenizer_context(const char * filename);
void destroy_tokenizer_context(struct tokenizer_context * context);

int get_one_char(struct tokenizer_context * context);
void unget_one_char(struct tokenizer_context * context, int ch);
int peek_one_char(struct tokenizer_context * context);

struct token * get_one_token(struct tokenizer_context * context);
void unget_one_token(struct tokenizer_context * context, struct token * token);
struct token * peek_one_token(struct tokenizer_context * context);

bool is_token_directive_equals(struct token * token, const char * directive_name);
bool is_token_punctuator(struct token * token, int ch);

#endif /* JCUNIT_TOKEN_H */
