#ifndef JCUNIT_DIFF_H
#define JCUNIT_DIFF_H 1

#include <stdio.h>

#include "string.h"

void print_diff(FILE * output, struct string * expected, struct string * actual);

#endif /* JCUNIT_DIFF_H */
