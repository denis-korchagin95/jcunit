#ifndef JCUNIT_ALLOCATE_H
#define JCUNIT_ALLOCATE_H 1

#include "token.h"
#include "string.h"
#include "ast.h"
#include "assembler.h"
#include "runner.h"

#define MAX_BYTES_POOL_SIZE (8192)

#define SHOW_STAT_ALL_ALLOCATORS (0)

#define allocator(name, type, count)                                                                \
    void * name##_free_list = NULL;                                                                 \
    static unsigned int max_##name##_pool_size = (count);                                           \
    static type name##_pool[(count)];                                                               \
    static unsigned int name##_pool_pos = 0;                                                        \
    static unsigned int name##_freed = 0;                                                           \
                                                                                                    \
    type * alloc_##name(void)                                                                       \
    {                                                                                               \
        if (name##_free_list != NULL) {                                                             \
            void ** list = (void**)name##_free_list;                                                \
            void * next = *list;                                                                    \
            *list = NULL;                                                                           \
            name##_free_list = next;                                                                \
            return (void *)list;                                                                    \
        }                                                                                           \
        if (name##_pool_pos >= (count)) {                                                           \
            fprintf(stderr, "Allocator " #name ": out of memory!\n");                               \
            exit(1);                                                                                \
        }                                                                                           \
        return &name##_pool[name##_pool_pos++];                                                     \
    }                                                                                               \
                                                                                                    \
    void free_##name(type * element)                                                                \
    {                                                                                               \
        static unsigned int pointer_size = (unsigned int)sizeof(void *);                            \
        static unsigned int element_size = (unsigned int)sizeof(type);                              \
        if (element_size < pointer_size) {                                                          \
            fprintf(                                                                                \
                stderr,                                                                             \
                "Cannot free element with size less than size of pointer! (pointer size: %u)\n",    \
                pointer_size                                                                        \
            );                                                                                      \
            exit(1);                                                                                \
        }                                                                                           \
        void ** list = (void **)element;                                                            \
        *list = name##_free_list;                                                                   \
        name##_free_list = (void *)element;                                                         \
        ++name##_freed;                                                                             \
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

void show_allocator_stats(FILE * output, unsigned int allocator);

#endif /* JCUNIT_ALLOCATE_H */
