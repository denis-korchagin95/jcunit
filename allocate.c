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
#include <assert.h>
#include <stdlib.h>


#include "headers/allocate.h"


/**
 * It's very simple and static allocator. Maybe this needs to be refactor to dynamic version in the future...
 */

struct allocator_stat {
    const char * name;
    unsigned int * total;
    unsigned int * pool_usage;
    unsigned int * freed;
    unsigned int * allocated;
};

static unsigned char bytes_pool[MAX_BYTES_POOL_SIZE] = {0};
static unsigned int bytes_pool_pos = 0;

allocator(token, struct token, 512)
allocator(string, struct string, 512)
allocator(tokenizer_context, struct tokenizer_context, 4)
allocator(ast_test_case, struct ast_test_case, 256)
allocator(ast_requirement, struct ast_requirement, 512)
allocator(list, struct list, 512)
allocator(test_case, struct test_case, 256)
allocator(requirement, struct requirement, 512)
allocator(test, struct test, 32)
allocator(test_result, struct test_result, 256)
allocator(test_case_result, struct test_case_result, 256)

static struct allocator_stat stats[] = {
        { "token", &max_token_pool_size, &token_pool_pos, &token_freed, &token_allocated },
        { "string", &max_string_pool_size, &string_pool_pos, &string_freed, &string_allocated },
        {
            "tokenizer_context",
            &max_tokenizer_context_pool_size,
            &tokenizer_context_pool_pos,
            &tokenizer_context_freed,
            &tokenizer_context_allocated
        },
        {
            "ast_test_case",
            &max_ast_test_case_pool_size,
            &ast_test_case_pool_pos,
            &ast_test_case_freed,
            &ast_test_case_allocated
        },
        {
            "ast_requirement",
            &max_ast_requirement_pool_size,
            &ast_requirement_pool_pos,
            &ast_requirement_freed,
            &ast_requirement_allocated
        },
        { "list", &max_list_pool_size, &list_pool_pos, &list_freed, &list_allocated },
        { "test_case", &max_test_case_pool_size, &test_case_pool_pos, &test_case_freed, &test_case_allocated },
        {
            "requirement",
            &max_requirement_pool_size,
            &requirement_pool_pos,
            &requirement_freed,
            &requirement_allocated
        },
        { "test", &max_test_pool_size, &test_pool_pos, &test_freed, &test_allocated },
        {
                "test_result",
                &max_test_result_pool_size,
                &test_result_pool_pos,
                &test_result_freed,
                &test_result_allocated
        },
        {
            "test_case_result",
            &max_test_case_result_pool_size,
            &test_case_result_pool_pos,
            &test_case_result_freed,
            &test_case_result_allocated
        },
};


void * alloc_bytes(unsigned int len)
{
    assert(bytes_pool_pos + len < MAX_BYTES_POOL_SIZE);
    void * mem = bytes_pool + bytes_pool_pos;
    bytes_pool_pos += len;
    return mem;
}

void show_allocator_stats(FILE * output, unsigned int allocator)
{
    unsigned int i, len;
    struct allocator_stat * stat;
    for(i = 0, len = sizeof(stats) / sizeof(struct allocator_stat); i < len; ++i) {
        stat = stats + i;
        fprintf(
            output,
            "Allocator: %s, total: %u, pool_usage: %u, allocated: %u, freed: %u\n",
            stat->name,
            *stat->total,
            *stat->pool_usage,
            *stat->allocated,
            *stat->freed
        );
    }
    unsigned int max_bytes_pool_size = MAX_BYTES_POOL_SIZE;
    fprintf(output, "Allocator: bytes, total: %u, pool_usage: %u\n", max_bytes_pool_size, bytes_pool_pos);
    fflush(output);
}
