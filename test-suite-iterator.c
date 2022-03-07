#include <stddef.h>
#include <assert.h>

#include "headers/test-suite-iterator.h"

void test_suite_iterator_init(struct test_suite_iterator * iterator, struct test_suite * test_suite)
{
    iterator->tests_count = test_suite->tests_count;
    iterator->tests = test_suite->tests;
    iterator->cursor = 0;
}

struct abstract_test_result * test_suite_iterator_visit(
    struct test_suite_iterator * iterator,
    test_suite_iterator_visiter_func * visiter_func,
    struct test_suite_result * context
) {
    assert(iterator != NULL);
    assert(visiter_func != NULL);

    if (test_suite_iterator_finished(iterator)) {
        return NULL;
    }

    void * result = visiter_func(test_suite_iterator_current(iterator), context, iterator->cursor);
    test_suite_iterator_next(iterator);
    return result;
}
