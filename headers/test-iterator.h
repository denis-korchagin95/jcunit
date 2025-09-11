#ifndef JCUNIT_TEST_ITERATOR_H
#define JCUNIT_TEST_ITERATOR_H 1

#include "compiler.h"

#define test_iterator_finished(iterator) ((iterator)->cursor >= (iterator)->tests_count)
#define test_iterator_current(iterator) ((iterator)->tests[(iterator)->cursor])
#define test_iterator_next(iterator) (++(iterator)->cursor)
#define test_iterator_current_safe(iterator) (test_iterator_finished(iterator) ? NULL : test_iterator_current(iterator))

struct tests_results;

struct test_iterator
{
    struct abstract_test ** tests;
    unsigned int tests_count;
    unsigned int cursor;
};

typedef struct abstract_test_result * test_iterator_visiter_func(
    struct abstract_test * test,
    struct tests_results * tests_results,
    unsigned int current_index
);

void test_iterator_init_by_suites(
    struct test_iterator * iterator,
    struct test_suite ** suites,
    unsigned int suites_count
);

void test_iterator_destroy(struct test_iterator * iterator);

struct abstract_test_result * test_iterator_visit(
    struct test_iterator * iterator,
    test_iterator_visiter_func * visiter_func,
    struct tests_results * tests_results
);

unsigned int get_total_tests_count(struct test_suite ** suites, unsigned int suites_count);

#endif /* JCUNIT_TEST_ITERATOR_H */
