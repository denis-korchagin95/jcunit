#ifndef JCUNIT_AST_H
#define JCUNIT_AST_H 1

#include "list.h"
#include "string.h"

#define REQUIREMENT_TYPE_ABSTRACT (1)

struct test
{
    struct list cases;
};

struct test_case
{
    struct list list_entry;
    struct list requirements;
    struct string * name;
};

struct abstract_requirement
{
    struct string * name;
    struct string * content;
};

struct requirement
{
    struct list list_entry;
    unsigned int type;
    void * instance;
};

#endif /* JCUNIT_AST_H */
