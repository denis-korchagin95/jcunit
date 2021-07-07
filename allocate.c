#include <assert.h>


#include "headers/allocate.h"


static unsigned char bytes_pool[MAX_BYTES_POOL_SIZE] = {0};
static unsigned int bytes_pool_pos = 0;


allocator(token, struct token, 512)
allocator(string, struct string, 512)
allocator(tokenizer_context, struct tokenizer_context, 4)


void * alloc_bytes(unsigned int len)
{
    assert(bytes_pool_pos + len < MAX_BYTES_POOL_SIZE);
    void * mem = bytes_pool + bytes_pool_pos;
    bytes_pool_pos += len;
    return mem;
}
