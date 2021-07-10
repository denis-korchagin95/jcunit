#ifndef JCUNIT_ASSEMBLER_H
#define JCUNIT_ASSEMBLER_H 1

#include "list.h"
#include "ast.h"

#define REQUIREMENT_ID_GIVEN_FILE     (1)
#define REQUIREMENT_ID_WHEN_RUN       (2)
#define REQUIREMENT_ID_EXPECT_OUTPUT  (3)

#define REQUIREMENT_KIND_GIVEN  (1)
#define REQUIREMENT_KIND_WHEN   (2)
#define REQUIREMENT_KIND_THEN   (3)

#define TEST_CASE_FLAG_HAS_GIVEN (1)
#define TEST_CASE_FLAG_HAS_WHEN  (2)
#define TEST_CASE_FLAG_HAS_THEN  (4)

#define TEST_CASE_COMPLETE_MASK (TEST_CASE_FLAG_HAS_GIVEN | TEST_CASE_FLAG_HAS_WHEN | TEST_CASE_FLAG_HAS_THEN)

#define REQUIREMENT_EXPECT_OUTPUT_RESOURCE_NONE   (-1)
#define REQUIREMENT_EXPECT_OUTPUT_RESOURCE_STDOUT (1)
#define REQUIREMENT_EXPECT_OUTPUT_RESOURCE_STDERR (2)

struct test {
    struct list cases;
    unsigned int case_count;
};

struct test_case {
    struct list list_entry;
    struct string * name;
    struct list requirements;
    unsigned int flags;
};

struct requirement {
    struct list list_entry;
    unsigned int id;
    unsigned int kind;
    struct string * content;
    union {
        struct string * path_to_executable;
        int resource_code;
    } extra;
};

struct test * assemble_test(struct list * ast_test_cases);

#endif /* JCUNIT_ASSEMBLER_H */
