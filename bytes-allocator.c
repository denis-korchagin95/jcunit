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
#include "headers/object-allocator.h"
#include "headers/errors.h"


#define MAX_BYTES_POOL_SIZE             (64 * 1024)
#define BYTES_CHUNK_HEADER_SIGNATURE    (0x8641ca7f)
#define BYTES_CHUNK_HEADER_SIZE         (sizeof(struct bytes_chunk_header))
#define WORD_SIZE                       (sizeof(void *))


#define bytes_chunk_data(bs)    ((void *)(bs) + BYTES_CHUNK_HEADER_SIZE)
#define bytes_chunk_offset(bs)  ((unsigned int)((bs)->chunk_size + (bs)->alignment + BYTES_CHUNK_HEADER_SIZE))

#define bytes_chunks_foreach(chunk_name, body)                                                                          \
    do                                                                                                                  \
    {                                                                                                                   \
        unsigned int ___cursor;                                                                                         \
        struct bytes_chunk_header * chunk_name = NULL;                                                                  \
        for (___cursor = 0; ___cursor < bytes_pool_pos; ___cursor += bytes_chunk_offset(chunk_name)) {                  \
            chunk_name = (struct bytes_chunk_header *)((void *)bytes_pool + ___cursor);                                 \
            if (chunk_name->signature != BYTES_CHUNK_HEADER_SIGNATURE)                                                  \
                break;                                                                                                  \
            body                                                                                                        \
        }                                                                                                               \
    }                                                                                                                   \
    while(0)


struct bytes_chunk_header
{
    unsigned int signature;
    unsigned int alignment:31, is_busy:1;
    unsigned int chunk_size;
};


static unsigned char bytes_pool[MAX_BYTES_POOL_SIZE] = {0};
static unsigned int bytes_pool_pos = 0;
static unsigned int allocated_bytes = 0;
static unsigned int freed_bytes = 0;
static void * bytes_pool_low_bound = (void *)bytes_pool + BYTES_CHUNK_HEADER_SIZE;
static void * bytes_pool_high_bound = (void *)bytes_pool + MAX_BYTES_POOL_SIZE - BYTES_CHUNK_HEADER_SIZE;


struct bytes_chunk_header * find_free_chunk_of_size(unsigned int len)
{
    bytes_chunks_foreach(chunk, {
        if (chunk->is_busy == 1)
            continue;
        if (chunk->chunk_size >= len)
            return chunk;
    });
    return NULL;
}


struct bytes_chunk_header * try_to_allocate_new_chunk(unsigned int len)
{
    unsigned int full_chunk_size = len + BYTES_CHUNK_HEADER_SIZE;
    unsigned int alignment = full_chunk_size % WORD_SIZE;
    if (alignment != 0)
        alignment = WORD_SIZE - alignment;
    full_chunk_size += alignment;
    if (bytes_pool_pos + full_chunk_size >= MAX_BYTES_POOL_SIZE) {
        jcunit_fatal_error("Allocator bytes: out of memory!");
    }
    struct bytes_chunk_header * chunk = (struct bytes_chunk_header *)(bytes_pool + bytes_pool_pos);
    chunk->signature = BYTES_CHUNK_HEADER_SIGNATURE;
    chunk->alignment = alignment;
    chunk->chunk_size = len;
    bytes_pool_pos += full_chunk_size;
    return chunk;
}


void * alloc_bytes(unsigned int len)
{
    struct bytes_chunk_header * chunk = find_free_chunk_of_size(len);
    if (chunk == NULL) {
        chunk = try_to_allocate_new_chunk(len);
    }
    chunk->is_busy = 1;
    allocated_bytes += chunk->chunk_size;
    return bytes_chunk_data(chunk);
}


void free_bytes(void * mem)
{
    if (mem < bytes_pool_low_bound || mem > bytes_pool_high_bound) {
        jcunit_fatal_error("Allocator bytes: try to free memory which out of allocator's bound!\n");
    }
    struct bytes_chunk_header * chunk = (struct bytes_chunk_header *)(mem - BYTES_CHUNK_HEADER_SIZE);
    if (chunk->signature != BYTES_CHUNK_HEADER_SIGNATURE)
        jcunit_fatal_error("Allocator bytes: try to free not managed memory!\n");
    chunk->is_busy = 0;
    freed_bytes += chunk->chunk_size;
}


void show_bytes_allocator_stats(FILE * output)
{
    unsigned int max_bytes_pool_size = MAX_BYTES_POOL_SIZE;
    fprintf(
        output,
        "Allocator: bytes, total: %u, pool_usage: %u, allocated: %u, freed: %u\n",
        max_bytes_pool_size,
        bytes_pool_pos,
        allocated_bytes,
        freed_bytes
    );
    unsigned int chunk_count = 0;
    unsigned int mem_size_in_chunks = 0;
    unsigned int mem_for_align_chunks = 0;
    bytes_chunks_foreach(chunk, {
        mem_size_in_chunks += chunk->chunk_size;
        mem_for_align_chunks += chunk->alignment;
        ++chunk_count;
    });
    unsigned int chunks_occupied_mem = chunk_count * (unsigned int)BYTES_CHUNK_HEADER_SIZE;
    fprintf(
        output,
        "\tchunk descriptor size: %lu, chunk_count: %u, chunks occupied mem: %u, mem size in chunks: %u, mem for align chunks: %u\n",
        BYTES_CHUNK_HEADER_SIZE,
        chunk_count,
        chunks_occupied_mem,
        mem_size_in_chunks,
        mem_for_align_chunks
    );
    fprintf(output, "\tchunk map: [");
    bytes_chunks_foreach(chunk, {
        fprintf(output, "%c", chunk->is_busy ? 'C' : 'F');
    });
    fprintf(output, "]\n");
    fflush(output);
}
