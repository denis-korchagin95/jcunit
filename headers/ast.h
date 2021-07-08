#ifndef JCUNIT_AST_H
#define JCUNIT_AST_H 1

#include "list.h"
#include "string.h"

struct ast_test_case
{
    struct list list_entry;
    struct list requirements;
    struct string * name;
};

struct ast_requirement
{
    struct list list_entry;
    struct string * name;
    struct string * argument;
    struct string * content;
};


struct ast_test_case * make_ast_test_case(void);
struct ast_requirement * make_ast_requirement(void);

#endif /* JCUNIT_AST_H */
