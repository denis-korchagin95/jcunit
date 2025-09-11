#include <stddef.h>
#include <assert.h>

#include "headers/test-iterator.h"
#include "headers/allocator.h"

void test_iterator_init_by_suites(
    struct test_iterator * iterator,
    struct test_suite ** suites,
    unsigned int suites_count
) {
    unsigned int total_tests_count = get_total_tests_count(suites, suites_count);
    iterator->tests = (struct abstract_test **) memory_blob_pool_alloc(&temporary_pool, total_tests_count * sizeof(void *));
    /* iterator initialization */
    {
        unsigned int suite_iterator, test_iterator, tests_count;
        unsigned int tests_offset = 0;
        for (suite_iterator = 0; suite_iterator < suites_count; ++suite_iterator) {
            struct test_suite * suite = suites[suite_iterator];
            for (test_iterator = 0, tests_count = suite->tests_count; test_iterator < tests_count; ++test_iterator) {
                iterator->tests[tests_offset++] = suite->tests[test_iterator];
            }
        }
    }
    iterator->tests_count = total_tests_count;
    iterator->cursor = 0;
}

void test_iterator_destroy(struct test_iterator * iterator)
{
    assert(iterator != NULL);
    if (iterator->tests != NULL) {
        iterator->tests = NULL;
    }
}

struct abstract_test_result * test_iterator_visit(
    struct test_iterator * iterator,
    test_iterator_visiter_func * visiter_func,
    struct tests_results * tests_results
) {
    assert(iterator != NULL);
    assert(visiter_func != NULL);

    if (test_iterator_finished(iterator)) {
        return NULL;
    }

    void * result = visiter_func(test_iterator_current(iterator), tests_results, iterator->cursor);
    test_iterator_next(iterator);
    return result;
}

unsigned int get_total_tests_count(struct test_suite ** suites, unsigned int suites_count)
{
    unsigned int tests_count = 0;
    unsigned int i;

    for (i = 0; i < suites_count; ++i) {
        const struct test_suite * suite = suites[i];

        tests_count += suite->tests_count;
    }

    return tests_count;
}
