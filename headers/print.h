#ifndef JCUNIT_PRINT_H
#define JCUNIT_PRINT_H 1

#include <stdio.h>

#include "token.h"
#include "ast.h"

void print_token(struct token * token, FILE * output);
void print_ast_test_case(struct ast_test_case * test_case, FILE * output);
void print_ast_requirement(struct ast_requirement * requirement, FILE * output);

#endif /* JCUNIT_PRINT_H */
