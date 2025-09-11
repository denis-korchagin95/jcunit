#ifndef JCUNIT_COMPILER_H
#define JCUNIT_COMPILER_H 1

#define TEST_FLAG_HAS_GIVEN     (1)
#define TEST_FLAG_HAS_WHEN      (2)
#define TEST_FLAG_HAS_THEN      (4)
#define TEST_FLAG_INCOMPLETE    (8)
#define TEST_FLAG_SKIPPED       (16)

#define TEST_KIND_PROGRAM_RUNNER (1)

#define TEST_COMPLETE_MASK (TEST_FLAG_HAS_GIVEN | TEST_FLAG_HAS_WHEN | TEST_FLAG_HAS_THEN)

#define TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_UNKNOWN    (0)
#define TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDOUT     (1)
#define TEST_PROGRAM_RUNNER_EXPECT_OUTPUT_STREAM_STDERR     (2)

struct test_suite
{
    struct source * source;
    const char * name;
    struct abstract_test ** tests;
    unsigned int tests_count;
};

struct abstract_test {
    struct string * name;
    struct test_suite * test_suite;
    unsigned int kind;
    unsigned int flags;
};

struct program_runner_test {
    struct abstract_test base;         /* this must be the first member */
    struct string * given_filename;
    struct string * given_file_content;
    struct string * program_path;
    struct string * program_args;
    struct string * expected_output;
    unsigned int stream_code;
};

struct test_suite * compile_test_suite(struct source * source);

#endif /* JCUNIT_COMPILER_H */
