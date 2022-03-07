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
#ifndef JCUNIT_SHOW_RESULT_H
#define JCUNIT_SHOW_RESULT_H 1

#include "runner.h"
#include "test-suite-iterator.h"

void show_test_result_in_detail_mode(struct abstract_test_result * test_result, FILE * output);
void show_test_result_in_passthrough_mode(struct abstract_test_result * test_result, FILE * output);

void show_each_test_result_in_detail_mode(
    FILE * output,
    struct test_suite_iterator * iterator,
    test_suite_iterator_visiter_func * visiter_func,
    void * context
);
void show_each_test_result_in_passthrough_mode(
    FILE * output,
    struct test_suite_iterator * iterator,
    test_suite_iterator_visiter_func * visiter_func,
    void * context
);

void * test_runner(void * object, void * context, unsigned int current_index);
void show_error_test_result(FILE * output, struct abstract_test_result * test_result, unsigned int error_number);

#endif /* JCUNIT_SHOW_RESULT_H */
