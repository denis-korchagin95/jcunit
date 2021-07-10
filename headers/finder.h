#ifndef JCUNIT_FINDER_H
#define JCUNIT_FINDER_H 1

#include "assembler.h"

struct requirement * find_requirement_by_kind(struct test_case * test_case, unsigned int kind);

#endif /* JCUNIT_FINDER_H */
