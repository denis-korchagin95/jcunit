#include <memory.h>


#include "headers/ast.h"
#include "headers/allocate.h"


struct ast_test_case * make_ast_test_case(void)
{
    struct ast_test_case * ast_test_case = alloc_ast_test_case();
    list_init(&ast_test_case->requirements);
    memset((void *)&ast_test_case->list_entry, 0, sizeof(struct list));
    ast_test_case->name = NULL;
    return ast_test_case;
}

struct ast_requirement * make_ast_requirement(void)
{
    struct ast_requirement * requirement = alloc_ast_requirement();
    memset((void *)&requirement->list_entry, 0, sizeof(struct list));
    requirement->name = NULL;
    requirement->argument = NULL;
    requirement->content = NULL;
    return requirement;
}
