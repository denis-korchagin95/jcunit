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
#ifndef JCUNIT_APPLICATION_H
#define JCUNIT_APPLICATION_H 1

#include <stdio.h>

#include "list.h"

#define RUN_MODE_PASSTHROUGH    (1)
#define RUN_MODE_DETAIL         (2)

#define OPTION_SHOW_ALLOCATORS_STATS            (1)
#define OPTION_SHOW_ALLOCATORS_STATS_LEAK_ONLY  (2)
#define OPTION_SHOW_VERSION                     (4)
#define OPTION_SHOW_HELP                        (8)

struct tests_results;

struct application_context
{
    unsigned int run_mode;
    unsigned int options;
};

struct test_suites
{
    struct test_suite ** suites;
    unsigned int suites_count;
};

void init_application_context(struct application_context * application_context);
void fetch_sources(int argc, char * argv[], struct slist * sources);
void read_suites(
    struct slist * sources,
    struct test_suites * test_suites
);
void run_suites(
    struct test_suites * test_suites,
    struct tests_results ** tests_results,
    struct application_context * application_context,
    FILE * output
);

#endif /* JCUNIT_APPLICATION_H */
