#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


#include "headers/token.h"
#include "headers/allocate.h"
#include "headers/string.h"


struct token newline_token = {0}, eof_token = {0}, character_token = {0};


struct token * get_one_punctuator(struct tokenizer_context * context, int ch)
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
        *w++ = ch;
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

struct tokenizer_context * make_tokenizer_context(FILE * source)
{
    struct tokenizer_context * context = alloc_tokenizer_context();
    context->source = source;
    context->token_buffer[0] = NULL;
    context->token_buffer_pos = 0;
    context->char_buffer[0] = '\0';
    context->char_buffer_pos = 0;
    context->mode = 0;
    return context;
}

void init_tokenizer(void)
{
    struct token * token;

    token = &eof_token;
    token->kind = TOKEN_KIND_EOF;
    token->content.ch = EOF;

    token = &character_token;
    token->kind = TOKEN_KIND_CHARACTER;

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
        context->char_buffer[context->char_buffer_pos++] = ch;
}

struct token * get_one_directive(struct tokenizer_context * context, int ch)
{
    static char buffer[MAX_NAME_LEN] = {0};

    char * w = buffer;

    ch = get_one_char(context);

    while (is_name_char(ch)) {
        *w++ = ch;
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
        }
        unget_one_char(context, next);
        goto character;
    }

    if (context->mode == TOKENIZER_MODE_DIRECTIVE_ONLY) {
        goto character;
    }

    if (ch == '\n') {
        return &newline_token;
    }

    if (ch == '(') {
        return get_one_punctuator(context, ch);
    }

    if (ch == ')') {
        return get_one_punctuator(context, ch);
    }

    if (ch == '"') {
        return get_one_string(context, ch);
    }

    character:
    character_token.content.ch = ch;
    return &character_token;
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

bool is_token_directive(struct token * token, const char * name)
{
    struct string * string = token->content.string;
    unsigned int len = strlen(name);
    return  token->kind == TOKEN_KIND_DIRECTIVE &&
            len == string->len &&
            strncmp(string->value, name, len) == 0;
}

bool is_token_punctuator(struct token * token, int ch)
{
    if (token->kind != TOKEN_KIND_PUNCTUATOR)
        return false;
    return token->content.ch == ch;
}
