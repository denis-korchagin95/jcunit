#include <memory.h>


#include "headers/ast.h"
#include "headers/allocator.h"


struct ast_test * make_ast_test(void)
{
    struct ast_test * ast_test = memory_blob_pool_alloc(&temporary_pool, sizeof(struct ast_test));
    memset(ast_test, 0, sizeof(struct ast_test));
    slist_init(&ast_test->requirements);
    slist_init(&ast_test->arguments);
    return ast_test;
}

struct ast_requirement * make_ast_requirement(void)
{
    struct ast_requirement * requirement = memory_blob_pool_alloc(&temporary_pool, sizeof(struct ast_requirement));
    memset(requirement, 0, sizeof(struct ast_requirement));
    slist_init(&requirement->arguments);
    return requirement;
}
