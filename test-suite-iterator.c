#include <stddef.h>
#include <assert.h>

#include "headers/test-suite-iterator.h"

void test_suite_iterator_init(struct test_suite_iterator * iterator, struct test_suite * test_suite)
{
    iterator->tests_count = test_suite->tests_count;
    iterator->tests = test_suite->tests;
    iterator->cursor = 0;
}

void * test_suite_iterator_visit(
    struct test_suite_iterator * iterator,
    test_suite_iterator_visiter_func * visiter_func,
    void * context
) {
    assert(iterator != NULL);
    assert(visiter_func != NULL);

    if (test_suite_iterator_finished(iterator)) {
        return NULL;
    }

    void * result = visiter_func((void *)test_suite_iterator_current(iterator), context);
    test_suite_iterator_next(iterator);
    return result;
}
