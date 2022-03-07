#include <stddef.h>
#include <assert.h>

#include "headers/test-iterator.h"

void test_iterator_init(struct test_iterator * iterator, struct test_suite * test_suite)
{
    iterator->tests_count = test_suite->tests_count;
    iterator->tests = test_suite->tests;
    iterator->cursor = 0;
}

struct abstract_test_result * test_iterator_visit(
    struct test_iterator * iterator,
    test_iterator_visiter_func * visiter_func,
    struct test_suite_result * context
) {
    assert(iterator != NULL);
    assert(visiter_func != NULL);

    if (test_iterator_finished(iterator)) {
        return NULL;
    }

    void * result = visiter_func(test_iterator_current(iterator), context, iterator->cursor);
    test_iterator_next(iterator);
    return result;
}
