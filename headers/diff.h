#ifndef JCUNIT_DIFF_H
#define JCUNIT_DIFF_H 1

#include <stdio.h>

#include "string.h"

extern int diff_use_colors;

void print_diff(FILE * output, struct string * expected, struct string * actual);

#endif /* JCUNIT_DIFF_H */
