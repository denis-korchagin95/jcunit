#ifndef JCUNIT_ALLOCATE_H
#define JCUNIT_ALLOCATE_H 1

#include "token.h"
#include "string.h"
#include "ast.h"

#define MAX_BYTES_POOL_SIZE (8192)

#define allocator(name, type, count)                \
    static type name##_pool[(count)];               \
    static unsigned int name##_pool_pos = 0;        \
                                                    \
    type * alloc_##name(void)                \
    {                                               \
        assert(name##_pool_pos < (count));          \
        return &name##_pool[name##_pool_pos++];     \
    }

#define declare_allocator(name, type) type * alloc_##name(void)

declare_allocator(tokenizer_context, struct tokenizer_context);
declare_allocator(token, struct token);
declare_allocator(string, struct string);
declare_allocator(test, struct test);
declare_allocator(test_case, struct test_case);

void * alloc_bytes(unsigned int len);

#endif /* JCUNIT_ALLOCATE_H */
