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
#include <stdio.h>
#include <stdlib.h>

#include "headers/token.h"
#include "headers/allocate.h"
#include "headers/version.h"
#include "headers/options.h"
#include "headers/application.h"

int main(int argc, char * argv[])
{
    struct application_context application_context;
    init_application_context(&application_context);

    parse_options(argc, argv, &application_context);

    if (option_show_version) {
        fprintf(stdout, "jcunit version %s\n", JCUNIT_VERSION);
        exit(0);
    }

    fetch_sources(argc, argv, &application_context);

    init_tokenizer();

    read_suites(&application_context);

    run_suites(stdout, &application_context);

    if (option_show_allocator_stats) {
        fprintf(stdout, "\n\n\n");

        show_allocators_stats(stdout);
    }

    fflush(stdout);

    return 0;
}
