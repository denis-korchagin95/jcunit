#ifndef JCUNIT_AST_H
#define JCUNIT_AST_H 1

#include "list.h"
#include "string.h"

struct ast_test
{
    struct slist list_entry;
    struct slist arguments;
    struct slist requirements;
};

struct ast_requirement_argument
{
    struct slist list_entry;
    struct string * name;
    struct string * value;
};

struct ast_requirement
{
    struct slist list_entry;
    struct slist arguments;
    struct string * name;
    struct string * content;
};


struct ast_test * make_ast_test(void);
struct ast_requirement * make_ast_requirement(void);

#endif /* JCUNIT_AST_H */
