#ifndef JCUNIT_PARSE_H
#define JCUNIT_PARSE_H 1

#include "token.h"
#include "list.h"

struct list * parse_test(struct tokenizer_context * context);

#endif /* JCUNIT_PARSE_H */
