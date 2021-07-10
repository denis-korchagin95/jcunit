#include <assert.h>


#include "headers/allocate.h"


/**
 * It's very simple and static allocator. Maybe this needs to be refactor to dynamic version in the future...
 */


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

void * alloc_bytes(unsigned int len)
{
    assert(bytes_pool_pos + len < MAX_BYTES_POOL_SIZE);
    void * mem = bytes_pool + bytes_pool_pos;
    bytes_pool_pos += len;
    return mem;
}
