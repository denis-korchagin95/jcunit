/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2021-2022 Denis Korchagin <denis.korchagin.1995@gmail.com>
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
#ifndef JCUNIT_TEST_ITERATOR_H
#define JCUNIT_TEST_ITERATOR_H 1

#include "compiler.h"

#define test_iterator_finished(iterator) ((iterator)->cursor >= (iterator)->tests_count)
#define test_iterator_current(iterator) ((iterator)->tests[(iterator)->cursor])
#define test_iterator_next(iterator) (++(iterator)->cursor)
#define test_iterator_current_safe(iterator) (test_iterator_finished(iterator) ? NULL : test_iterator_current(iterator))

struct tests_results;

struct test_iterator
{
    struct abstract_test ** tests;
    unsigned int tests_count;
    unsigned int cursor;
};

typedef struct abstract_test_result * test_iterator_visiter_func(
    struct abstract_test * test,
    struct tests_results * tests_results,
    unsigned int current_index
);

void test_iterator_init_by_suites(
    struct test_iterator * iterator,
    struct test_suite ** suites,
    unsigned int suites_count
);

void test_iterator_destroy(struct test_iterator * iterator);

struct abstract_test_result * test_iterator_visit(
    struct test_iterator * iterator,
    test_iterator_visiter_func * visiter_func,
    struct tests_results * tests_results
);

#endif /* JCUNIT_TEST_ITERATOR_H */
