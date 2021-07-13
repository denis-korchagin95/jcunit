/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2021 Denis Korchagin <denis.korchagin.1995@gmail.com>
 *
 * This file is part of JCUnit
 *
 * For the full license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation of version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


#include "headers/assembler.h"
#include "headers/allocate.h"
#include "headers/util.h"

static const char * test_case_flag_stringify(unsigned int flag);
static struct test * make_test(void);
static struct test_case * make_test_case(void);
static struct requirement * make_requirement(void);
static unsigned int get_test_case_flag_for_requirement(struct requirement * requirement);


static struct test_case * assemble_ast_test_case(struct ast_test_case * ast_test_case);
static struct requirement * assemble_ast_requirement(struct ast_requirement * ast_requirement);
static struct requirement * assemble_given_requirement(struct ast_requirement * ast_requirement);
static struct requirement * assemble_when_run_requirement(struct ast_requirement * ast_requirement);
static struct requirement * assemble_expect_output_requirement(struct ast_requirement * ast_requirement);


struct test * assemble_test(const char * filename, struct list * ast_test_cases)
{
    if (list_is_empty(ast_test_cases)) {
        fprintf(stderr, "There no any test case to assemble the test!\n");
        exit(1);
    }
    const char * test_name = basename(filename);
    if (test_name == NULL) {
        test_name = "<unnamed>";
    }

    struct test_case * test_case;
    struct ast_test_case * ast_test_case;

    struct test * test = make_test();
    test->name = make_string(test_name, strlen(test_name));

    list_foreach(iterator, ast_test_cases, {
        ast_test_case = list_get_owner(iterator, struct ast_test_case, list_entry);
        test_case = assemble_ast_test_case(ast_test_case);
        test_case->test = test;
        list_append(&test->cases, &test_case->list_entry);
        ++test->case_count;
    });
    return test;
}

struct test_case * assemble_ast_test_case(struct ast_test_case * ast_test_case)
{
    struct test_case * test_case = make_test_case();

    struct list * head = &ast_test_case->requirements;
    struct list * iterator = head->next;

    test_case->name = ast_test_case->name;

    while (iterator != head) {
        struct ast_requirement * ast_requirement = list_get_owner(iterator, struct ast_requirement, list_entry);
        struct requirement * requirement = assemble_ast_requirement(ast_requirement);
        unsigned int flag = get_test_case_flag_for_requirement(requirement);
        if (flag == 0) {
            fprintf(
                    stderr,
                    "The unknown way of the usage of the requirement \"%.*s\"!\n",
                    ast_requirement->name->len,
                    ast_requirement->name->value
            );
            exit(1);
        }
        if ((test_case->flags & flag) == flag) {
            fprintf(stderr, "The duplicate of the '%s' kind of requirements!\n", test_case_flag_stringify(flag));
            exit(1);
        }
        test_case->flags |= flag;
        list_append(&test_case->requirements, &requirement->list_entry);
        iterator = iterator->next;
    }

    if ((test_case->flags & TEST_CASE_COMPLETE_MASK) != TEST_CASE_COMPLETE_MASK) {
        const char * missing_kind_of_requirement = "<unknown kind of requirement>";
        if ((test_case->flags & TEST_CASE_FLAG_HAS_GIVEN) != TEST_CASE_FLAG_HAS_GIVEN) {
            missing_kind_of_requirement = "given";
        }
        if ((test_case->flags & TEST_CASE_FLAG_HAS_WHEN) != TEST_CASE_FLAG_HAS_WHEN) {
            missing_kind_of_requirement = "when";
        }
        if ((test_case->flags & TEST_CASE_FLAG_HAS_THEN) != TEST_CASE_FLAG_HAS_THEN) {
            missing_kind_of_requirement = "then";
        }
        fprintf(
                stderr,
                "The test case incomplete: missing the '%s' kind of requirement!\n",
                missing_kind_of_requirement
        );
        exit(1);
    }


    return test_case;
}

struct requirement * assemble_ast_requirement(struct ast_requirement * ast_requirement)
{
    if (string_equals_with_cstring(ast_requirement->name, "given")) {
        return assemble_given_requirement(ast_requirement);
    }
    if (string_equals_with_cstring(ast_requirement->name, "whenRun")) {
        return assemble_when_run_requirement(ast_requirement);
    }
    if (string_equals_with_cstring(ast_requirement->name, "expectOutput")) {
        return assemble_expect_output_requirement(ast_requirement);
    }
    fprintf(stderr, "The unknown \"%.*s\" requirement!\n", ast_requirement->name->len, ast_requirement->name->value);
    exit(1);
}

struct requirement * assemble_given_requirement(struct ast_requirement * ast_requirement)
{
    if (!string_equals_with_cstring(ast_requirement->argument, "file")) {
        fprintf(
                stderr,
                "The unknown \"%.*s\" given type!\n",
                ast_requirement->argument->len,
                ast_requirement->argument->value
        );
        exit(1);
    }
    struct requirement * requirement = make_requirement();
    requirement->kind = REQUIREMENT_KIND_GIVEN;
    requirement->id = REQUIREMENT_ID_GIVEN_FILE;
    requirement->content = ast_requirement->content;
    return requirement;
}

struct requirement * assemble_when_run_requirement(struct ast_requirement * ast_requirement)
{
    if (ast_requirement->content != NULL) {
        fprintf(stderr, "The 'whenRun' requirement must not contain any content!\n");
        exit(1);
    }
    struct requirement * requirement = make_requirement();
    requirement->kind = REQUIREMENT_KIND_WHEN;
    requirement->id = REQUIREMENT_ID_WHEN_RUN;
    requirement->extra.path_to_executable = ast_requirement->argument;
    return requirement;
}

struct requirement * assemble_expect_output_requirement(struct ast_requirement * ast_requirement)
{
    int resource_code = -1;
    if (string_equals_with_cstring(ast_requirement->argument, "stdout")) {
        resource_code = REQUIREMENT_EXPECT_OUTPUT_RESOURCE_STDOUT;
    }
    if (string_equals_with_cstring(ast_requirement->argument, "stderr")) {
        resource_code = REQUIREMENT_EXPECT_OUTPUT_RESOURCE_STDERR;
    }
    if (resource_code == -1) {
        fprintf(
                stderr,
                "The unknown resource \"%.*s\" for 'expectOutput' requirement!\n",
                ast_requirement->argument->len,
                ast_requirement->argument->value
        );
        exit(1);
    }
    struct requirement * requirement = make_requirement();
    requirement->id = REQUIREMENT_ID_EXPECT_OUTPUT;
    requirement->kind = REQUIREMENT_KIND_THEN;
    requirement->extra.resource_code = resource_code;
    requirement->content = ast_requirement->content;
    return requirement;
}

struct test_case * make_test_case(void)
{
    struct test_case * test_case = alloc_test_case();
    memset((void *)&test_case->list_entry, 0, sizeof(struct list));
    test_case->name = NULL;
    list_init(&test_case->requirements);
    test_case->flags = 0;
    test_case->test = NULL;
    return test_case;
}

struct requirement * make_requirement(void)
{
    struct requirement * requirement = alloc_requirement();
    memset((void *)&requirement->list_entry, 0, sizeof(struct list));
    requirement->content = NULL;
    requirement->kind = 0;
    memset((void *)&requirement->extra, 0, sizeof(requirement->extra));
    requirement->id = 0;
    return requirement;
}

struct test * make_test(void)
{
    struct test * test = alloc_test();
    list_init(&test->cases);
    test->case_count = 0;
    test->name = NULL;
    return test;
}

unsigned int get_test_case_flag_for_requirement(struct requirement * requirement)
{
    switch (requirement->kind) {
        case REQUIREMENT_KIND_GIVEN: return TEST_CASE_FLAG_HAS_GIVEN;
        case REQUIREMENT_KIND_THEN: return TEST_CASE_FLAG_HAS_THEN;
        case REQUIREMENT_KIND_WHEN: return TEST_CASE_FLAG_HAS_WHEN;
    }
    return 0;
}

const char * test_case_flag_stringify(unsigned int flag)
{
    static char buffer[100] = {0};
    switch (flag) {
        case TEST_CASE_FLAG_HAS_GIVEN:
            sprintf(buffer, "given");
            break;
        case TEST_CASE_FLAG_HAS_WHEN:
            sprintf(buffer, "when");
            break;
        case TEST_CASE_FLAG_HAS_THEN:
            sprintf(buffer, "then");
            break;
        default:
            fprintf(stderr, "The unknown test case flag \"%u\"\n!", flag);
            exit(1);
    }
    return buffer;
}
