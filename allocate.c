#include <assert.h>


#include "headers/allocate.h"


/**
 * It's very simple and static allocator. Maybe this needs to be refactor to dynamic version in the future...
 */

struct allocator_stat {
    const char * name;
    unsigned int * total;
    unsigned int * allocated;
    unsigned int * freed;
};

static unsigned char bytes_pool[MAX_BYTES_POOL_SIZE] = {0};
static unsigned int bytes_pool_pos = 0;

allocator(token, struct token, 512)
allocator(string, struct string, 512)
allocator(tokenizer_context, struct tokenizer_context, 4)
allocator(ast_test_case, struct ast_test_case, 256)
allocator(ast_requirement, struct ast_requirement, 512)
allocator(list, struct list, 512)
allocator(test_case, struct test_case, 256)
allocator(requirement, struct requirement, 512)
allocator(test, struct test, 32)
allocator(test_runner_context, struct test_runner_context, 4)
allocator(test_case_result, struct test_case_result, 256)

static struct allocator_stat stats[] = {
        { "token", &max_token_pool_size, &token_pool_pos, 0 },
        { "string", &max_string_pool_size, &string_pool_pos, 0 },
        { "tokenizer_context", &max_tokenizer_context_pool_size, &tokenizer_context_pool_pos, 0 },
        { "ast_test_case", &max_ast_test_case_pool_size, &ast_test_case_pool_pos, 0 },
        { "ast_requirement", &max_ast_requirement_pool_size, &ast_requirement_pool_pos, 0 },
        { "list", &max_list_pool_size, &list_pool_pos, 0 },
        { "test_case", &max_test_case_pool_size, &test_case_pool_pos, 0 },
        { "requirement", &max_requirement_pool_size, &requirement_pool_pos, 0 },
        { "test", &max_test_pool_size, &test_pool_pos, 0 },
        { "test_runner_context", &max_test_runner_context_pool_size, &test_runner_context_pool_pos, 0 },
        { "test_case_result", &max_test_case_result_pool_size, &test_case_result_pool_pos, 0 },
};


void * alloc_bytes(unsigned int len)
{
    assert(bytes_pool_pos + len < MAX_BYTES_POOL_SIZE);
    void * mem = bytes_pool + bytes_pool_pos;
    bytes_pool_pos += len;
    return mem;
}

void show_allocator_stats(FILE * output, unsigned int allocator)
{
    unsigned int i, len;
    struct allocator_stat * stat;
    for(i = 0, len = sizeof(stats) / sizeof(struct allocator_stat); i < len; ++i) {
        stat = stats + i;
        fprintf(output, "Allocator: %s, total: %u, allocated: %u\n", stat->name, *stat->total, *stat->allocated);
    }
}
