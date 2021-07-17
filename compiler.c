#include "headers/compiler.h"
#include "headers/token.h"
#include "headers/parse.h"
#include "headers/allocate.h"


static void release_ast_cases(struct slist * ast_cases);
static void release_ast_requirements(struct slist * ast_requirements);
static void release_ast_case(struct ast_test_case * ast_test_case);
static void release_ast_requirement(struct ast_requirement * requirement);


struct test * compile_test(const char * filename)
{
    struct tokenizer_context * context = make_tokenizer_context(filename);

    struct slist * ast_cases = parse_test(context);

    destroy_tokenizer_context(context);

    struct test * test = assemble_test(filename, ast_cases);

    release_ast_cases(ast_cases);
    free_slist(ast_cases);

    return test;
}

static void release_ast_cases(struct slist * ast_cases)
{
    slist_foreach_safe(iterator, ast_cases, {
        release_ast_case(list_get_owner(iterator, struct ast_test_case, list_entry));
    });
}

static void release_ast_case(struct ast_test_case * ast_test_case)
{
    release_ast_requirements(&ast_test_case->requirements);
    free_ast_test_case(ast_test_case);
}

void release_ast_requirements(struct slist * ast_requirements)
{
    slist_foreach_safe(iterator, ast_requirements, {
        release_ast_requirement(list_get_owner(iterator, struct ast_requirement, list_entry));
    });
}

void release_ast_requirement(struct ast_requirement * requirement)
{
    free_ast_requirement(requirement);
}
