#ifndef JCUNIT_TEST_ITERATOR_H
#define JCUNIT_TEST_ITERATOR_H 1

#include "compiler.h"

#define test_iterator_finished(iterator) ((iterator)->cursor >= (iterator)->tests_count)
#define test_iterator_current(iterator) ((iterator)->tests[(iterator)->cursor])
#define test_iterator_next(iterator) (++(iterator)->cursor)

struct test_suite_result;

struct test_iterator
{
    struct abstract_test ** tests;
    unsigned int tests_count;
    unsigned int cursor;
};

typedef struct abstract_test_result * test_iterator_visiter_func(
    struct abstract_test * test,
    struct test_suite_result * test_suite_result,
    unsigned int current_index
);

void test_iterator_init(struct test_iterator * iterator, struct test_suite * test_suite);
struct abstract_test_result * test_iterator_visit(
    struct test_iterator * iterator,
    test_iterator_visiter_func * visiter_func,
    struct test_suite_result * test_suite_result
);

#endif /* JCUNIT_TEST_ITERATOR_H */
