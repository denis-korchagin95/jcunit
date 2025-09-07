#ifndef JCUNIT_ALLOCATE_H
#define JCUNIT_ALLOCATE_H 1

#include "token.h"
#include "string.h"
#include "ast.h"
#include "compiler.h"
#include "runner.h"
#include "source.h"
#include "fs.h"
#include "application.h"

#define allocator(name, type, count)                                                                \
    void * name##_free_list = NULL;                                                                 \
    static unsigned int max_##name##_pool_size = (count);                                           \
    static type name##_pool[(count)];                                                               \
    static unsigned int name##_pool_pos = 0;                                                        \
    static unsigned int name##_freed = 0;                                                           \
    static unsigned int name##_allocated = 0;                                                       \
                                                                                                    \
    type * alloc_##name(void)                                                                       \
    {                                                                                               \
        ++name##_allocated;                                                                         \
        if (name##_free_list != NULL) {                                                             \
            void ** ptr = (void**)name##_free_list;                                                 \
            void * next = *ptr;                                                                     \
            void * object = (void*)name##_free_list;                                                \
            name##_free_list = next;                                                                \
            return object;                                                                          \
        }                                                                                           \
        if (name##_pool_pos >= (count)) {                                                           \
            jcunit_fatal_error("Allocator " #name ": out of memory!");                              \
        }                                                                                           \
        return &name##_pool[name##_pool_pos++];                                                     \
    }                                                                                               \
    void free_##name(type * object)                                                                 \
    {                                                                                               \
        static unsigned int pointer_size = (unsigned int)sizeof(void *);                            \
        static unsigned int object_size = (unsigned int)sizeof(type);                               \
        if (object_size < pointer_size) {                                                           \
            jcunit_fatal_error(                                                                     \
                "Cannot free element with size less than size of pointer! (pointer size: %u)",      \
                pointer_size                                                                        \
            );                                                                                      \
        }                                                                                           \
        void ** ptr = (void **)object;                                                              \
        *ptr = name##_free_list;                                                                    \
        name##_free_list = (void *)object;                                                          \
        ++name##_freed;                                                                             \
    }

#define declare_allocator(name, type)   \
    type * alloc_##name(void);          \
    void free_##name(type * object);

declare_allocator(tokenizer_context, struct tokenizer_context);
declare_allocator(token, struct token);
declare_allocator(string, struct string);
declare_allocator(ast_test, struct ast_test);
declare_allocator(ast_requirement, struct ast_requirement);
declare_allocator(list, struct list);
declare_allocator(slist, struct slist);
declare_allocator(test_suite, struct test_suite);
declare_allocator(tests_results, struct tests_results);
declare_allocator(abstract_test_result, struct abstract_test_result);
declare_allocator(program_runner_test_result, struct program_runner_test_result);
declare_allocator(program_runner_test, struct program_runner_test);
declare_allocator(source, struct source);
declare_allocator(path_list, struct path_list);
declare_allocator(ast_requirement_argument, struct ast_requirement_argument);

void show_allocators_stats(FILE * output, bool show_leak_only);

void release_ast_tests(struct slist * ast_tests);
void release_ast_requirements(struct slist * requirements);
void release_ast_requirement(struct ast_requirement * requirement);
void release_ast_test(struct ast_test * ast_test);
void release_ast_arguments(struct slist * arguments);
void release_ast_argument(struct ast_requirement_argument * argument);
void release_path_list(struct path_list * item);
void release_string(struct string * string);
void release_tests_results(struct tests_results * tests_results);
void release_test_result(struct abstract_test_result * test_result);
void release_test(struct abstract_test * test);
void release_test_suite(struct test_suite * test_suite);
void release_source(struct source * source);
void release_token(struct token * token);
void release_test_suites(struct test_suites * test_suites);

#endif /* JCUNIT_ALLOCATE_H */
