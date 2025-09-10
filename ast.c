#include <memory.h>


#include "headers/ast.h"
#include "headers/allocator.h"


struct ast_test * make_ast_test(void)
{
    main_pool_alloc(struct ast_test, ast_test)
    slist_init(&ast_test->requirements);
    slist_init(&ast_test->arguments);
    return ast_test;
}

struct ast_requirement * make_ast_requirement(void)
{
    main_pool_alloc(struct ast_requirement, requirement);
    slist_init(&requirement->arguments);
    return requirement;
}
