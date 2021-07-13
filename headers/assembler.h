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

#define REQUIREMENT_EXPECT_OUTPUT_RESOURCE_STDOUT (1)
#define REQUIREMENT_EXPECT_OUTPUT_RESOURCE_STDERR (2)

struct test {
    struct list cases;
    struct string * name;
    unsigned int case_count;
};

struct test_case {
    struct list list_entry;
    struct string * name;
    struct test * test;
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

struct test * assemble_test(const char * filename, struct list * ast_test_cases);

#endif /* JCUNIT_ASSEMBLER_H */
