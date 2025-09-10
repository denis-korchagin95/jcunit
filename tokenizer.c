#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "headers/token.h"
#include "headers/string.h"
#include "headers/errors.h"
#include "headers/allocator.h"


struct token newline_token = {0}, eof_token = {0};


struct token * make_punctuator_token(int ch)
{
    main_pool_alloc(struct token, token)
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
            jcunit_fatal_error("Unterminated string!");
        }
        if (ch == '\n') {
            jcunit_fatal_error("Newline inside the string!");
        }
        *w++ = (char)ch;
        ++len;
        ch = get_one_char(context);
    }
    if (len >= MAX_STRING_LEN) {
        jcunit_fatal_error("Too long string!");
    }
    main_pool_alloc(struct token, token)
    token->kind = TOKEN_KIND_STRING;
    token->content.string = make_string(buffer, len);
    return token;
}

struct tokenizer_context * make_tokenizer_context(const char * filename)
{
    FILE * source = fopen(filename, "r");
    if (source == NULL) {
        jcunit_fatal_error("Can't read file \"%s\": %s", filename, strerror(errno));
    }
    main_pool_alloc(struct tokenizer_context, context)
    context->filename = filename;
    context->source = source;
    return context;
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
        jcunit_fatal_error("The char buffer is overflow!");
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

struct token * get_one_name(struct tokenizer_context * context, int ch, unsigned int token_kind)
{
    static char buffer[MAX_NAME_LEN] = {0};

    char * w = buffer;

    unsigned int len = 0;

    for (;;) {
        *w++ = (char)ch;
        ++len;
        if (len >= MAX_NAME_LEN) {
            jcunit_fatal_error("Too long name '%.*s' exceed %d characters!", len, buffer, MAX_NAME_LEN);
        }
        ch = get_one_char(context);
        if (!is_name_char(ch)) {
            break;
        }
    }
    unget_one_char(context, ch);

    main_pool_alloc(struct token, token)
    token->kind = token_kind;
    token->content.string = make_string(buffer, len);
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
            return get_one_name(context, next, TOKEN_KIND_DIRECTIVE);
        }
        if (next == '@') {
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

    if (ch == ',') {
        return make_punctuator_token(ch);
    }

    if (ch == '(') {
        return make_punctuator_token(ch);
    }

    if (ch == ')') {
        return make_punctuator_token(ch);
    }

    if (ch == '=') {
        return make_punctuator_token(ch);
    }

    if (ch == '"') {
        return get_one_string(context, ch);
    }

    if (is_start_name_char(ch)) {
        return get_one_name(context, ch, TOKEN_KIND_NAME);
    }

    struct token * token;

character:
    token = memory_blob_pool_alloc(&permanent_pool, sizeof(struct token));
    memset(token, 0, sizeof(struct token));
    token->kind = TOKEN_KIND_CHARACTER;
    token->content.ch = ch;
    return token;
}

void unget_one_token(struct tokenizer_context * context, struct token * token)
{
    assert(token != NULL);
    if (context->token_buffer_pos >= MAX_TOKEN_BUFFER_SIZE) {
        jcunit_fatal_error("The token buffer is overflow!");
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
