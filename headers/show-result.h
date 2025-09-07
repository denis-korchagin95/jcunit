#ifndef JCUNIT_SHOW_RESULT_H
#define JCUNIT_SHOW_RESULT_H 1

#include <stdio.h>

#include "runner.h"
#include "test-iterator.h"

void show_test_result_in_detail_mode(struct abstract_test_result * test_result, FILE * output);
void show_test_result_in_passthrough_mode(struct abstract_test_result * test_result, FILE * output);

struct abstract_test_result * test_runner(
    struct abstract_test * test,
    struct tests_results * tests_results,
    unsigned int current_index
);
void show_error_test_result(FILE * output, struct abstract_test_result * test_result, unsigned int error_number);
void show_failure_test_result(FILE * output, struct abstract_test_result * test_result, unsigned int failure_number);

#endif /* JCUNIT_SHOW_RESULT_H */
