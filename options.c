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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "headers/options.h"

bool option_show_allocators_stats = false;
bool option_show_allocators_stats_leak_only = false;
bool option_show_version = false;


void parse_options(int argc, char * argv[], struct application_context * application_context)
{
    int i;
    char * arg;
    for(i = 1; i < argc; ++i) {
        arg = argv[i];
        if (strncmp("--show-allocators-stats-leak-only", arg, sizeof("--show-allocators-stats-leak-only") - 1) == 0) {
            option_show_allocators_stats = true;
            option_show_allocators_stats_leak_only = true;
            continue;
        }
        if (strncmp("--show-allocators-stats", arg, sizeof("--show-allocators-stats") - 1) == 0) {
            option_show_allocators_stats = true;
            continue;
        }
        if (strncmp("--version", arg, sizeof("--version") - 1) == 0) {
            option_show_version = true;
            continue;
        }
        if (strncmp("--run-mode=", arg, sizeof("--run-mode=") - 1) == 0) {
            const char * run_mode = arg + sizeof("--run-mode=") - 1;
            if (strcmp(run_mode, "detail") == 0) {
                application_context->run_mode = RUN_MODE_DETAIL;
                continue;
            }
            if (strcmp(run_mode, "passthrough") == 0) {
                application_context->run_mode = RUN_MODE_PASSTHROUGH;
                continue;
            }
            fprintf(stderr, "The unknown run mode '%s'!\n", run_mode);
            exit(1);
        }
        if (strncmp("--", arg, 2) == 0) {
            fprintf(stderr, "The unknown option: %s!\n", arg);
            exit(1);
        }
    }
}
