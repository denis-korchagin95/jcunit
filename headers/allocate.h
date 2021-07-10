#ifndef JCUNIT_ALLOCATE_H
#define JCUNIT_ALLOCATE_H 1

#include "token.h"
#include "string.h"
#include "ast.h"
#include "assembler.h"
#include "runner.h"

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
declare_allocator(ast_test_case, struct ast_test_case);
declare_allocator(ast_requirement, struct ast_requirement);
declare_allocator(list, struct list);
declare_allocator(test_case, struct test_case);
declare_allocator(requirement, struct requirement);
declare_allocator(test, struct test);
declare_allocator(test_runner_context, struct test_runner_context);
declare_allocator(test_case_result, struct test_case_result);

void * alloc_bytes(unsigned int len);

#endif /* JCUNIT_ALLOCATE_H */
