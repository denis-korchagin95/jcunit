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
#include <stdbool.h>

#include "headers/object-allocator.h"
#include "headers/errors.h"


#define MAX_BYTES_POOL_SIZE             (64 * 1024)
#define BYTES_CHUNK_HEADER_SIGNATURE    (0x8641ca7f)
#define BYTES_CHUNK_HEADER_SIZE         (sizeof(struct bytes_chunk_header))
#define WORD_SIZE                       (sizeof(void *))


#define bytes_chunk_data(bs)            ((void *)(bs) + BYTES_CHUNK_HEADER_SIZE)
#define bytes_chunk_offset(bs)          ((unsigned int)((bs)->size + (bs)->alignment + BYTES_CHUNK_HEADER_SIZE))
#define bytes_chunk_get(pool, offset)   ((struct bytes_chunk_header *)((void *)(pool + offset)))


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
    unsigned int size;
};

struct defragmentation_info
{
    struct bytes_chunk_header * first_free_chunk_pair;
    struct bytes_chunk_header * latest_free_chunk;
};


static unsigned char bytes_pool[MAX_BYTES_POOL_SIZE] = {0};
static unsigned int bytes_pool_pos = 0;
static unsigned int allocated_bytes = 0;
static unsigned int freed_bytes = 0;
static void * bytes_pool_low_bound = (void *)bytes_pool + BYTES_CHUNK_HEADER_SIZE;
static void * bytes_pool_high_bound = (void *)bytes_pool + MAX_BYTES_POOL_SIZE - BYTES_CHUNK_HEADER_SIZE;
static struct defragmentation_info defragmentation_info;

struct bytes_chunk_header * find_latest_free_chunk(unsigned int start_offset)
{
    unsigned int offset = start_offset;
    struct bytes_chunk_header * last_chunk = NULL;
    for (;;) {
        if (offset >= bytes_pool_pos)
            break;
        struct bytes_chunk_header * chunk = bytes_chunk_get(bytes_pool, offset);
        if (chunk->signature != BYTES_CHUNK_HEADER_SIGNATURE)
            break;
        last_chunk = chunk;
        offset += bytes_chunk_offset(chunk);
    }
    if (last_chunk != NULL && last_chunk->is_busy == 0) {
        return last_chunk;
    }
    return NULL;
}


struct bytes_chunk_header * find_first_free_pair(unsigned int start_offset)
{
    unsigned int offset = start_offset;
    for (;;) {
        if (offset >= bytes_pool_pos)
            break;
        struct bytes_chunk_header * chunk = bytes_chunk_get(bytes_pool, offset);
        if (chunk->signature != BYTES_CHUNK_HEADER_SIGNATURE)
            break;
        unsigned int next_chunk_offset = offset + bytes_chunk_offset(chunk);
        struct bytes_chunk_header * next_chunk = next_chunk_offset >= bytes_pool_pos
                ? NULL
                : bytes_chunk_get(bytes_pool, next_chunk_offset);
        if (next_chunk == NULL || next_chunk->signature != BYTES_CHUNK_HEADER_SIGNATURE)
            break;
        if (chunk->is_busy == 0 && next_chunk->is_busy == 0) {
            return chunk;
        }
        offset = next_chunk_offset + bytes_chunk_offset(next_chunk);
    }
    return NULL;
}


bool needs_defragmentation(struct defragmentation_info * defragmentation_info)
{
    defragmentation_info->first_free_chunk_pair = find_first_free_pair(0);
    defragmentation_info->latest_free_chunk = find_latest_free_chunk(0);
    return defragmentation_info->first_free_chunk_pair != NULL
        || defragmentation_info->latest_free_chunk != NULL;
}


void do_defragmentation(struct defragmentation_info * defragmentation_info)
{
    /* TODO: implement defragmentation of bytes chunks */
}


struct bytes_chunk_header * find_free_chunk_of_size(unsigned int len)
{
    bytes_chunks_foreach(chunk, {
        if (chunk->is_busy == 1)
            continue;
        if (chunk->size >= len)
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
    struct bytes_chunk_header * chunk = bytes_chunk_get(bytes_pool, bytes_pool_pos);
    chunk->signature = BYTES_CHUNK_HEADER_SIGNATURE;
    chunk->alignment = alignment;
    chunk->size = len;
    bytes_pool_pos += full_chunk_size;
    return chunk;
}


void * alloc_bytes(unsigned int len)
{
    defragmentation_info.latest_free_chunk = NULL;
    defragmentation_info.first_free_chunk_pair = NULL;
    if (needs_defragmentation(&defragmentation_info)) {
        do_defragmentation(&defragmentation_info);
    }
    struct bytes_chunk_header * chunk = find_free_chunk_of_size(len);
    if (chunk == NULL) {
        chunk = try_to_allocate_new_chunk(len);
    }
    chunk->is_busy = 1;
    allocated_bytes += chunk->size;
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
    freed_bytes += chunk->size;
}


void show_bytes_allocator_stats(FILE * output, bool show_leak_only)
{
    if (show_leak_only && !(allocated_bytes > freed_bytes)) {
        return;
    }
    unsigned int max_bytes_pool_size = MAX_BYTES_POOL_SIZE;
    const char * allocator_status = "OK";
    if (allocated_bytes < freed_bytes) {
        allocator_status = "ALLOCATION MISMATCH";
    } else if (allocated_bytes > freed_bytes) {
        allocator_status = "LEAK";
    }
    fprintf(
        output,
        "Allocator: bytes, total: %u, pool_usage: %u, allocated: %u, freed: %u [%s]\n",
        max_bytes_pool_size,
        bytes_pool_pos,
        allocated_bytes,
        freed_bytes,
        allocator_status
    );
    unsigned int chunk_count = 0;
    unsigned int mem_size_in_chunks = 0;
    unsigned int mem_for_align_chunks = 0;
    bytes_chunks_foreach(chunk, {
        mem_size_in_chunks += chunk->size;
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
    fprintf(output, "\tChunk Memory Map: [");
    unsigned int max_chunk_size = 0;
    unsigned int min_chunk_size = UINT32_MAX;
    unsigned int chunk_size_sum = 0;
    bytes_chunks_foreach(chunk, {
        fprintf(output, "%c", chunk->is_busy ? 'C' : 'F');
        if (chunk->size < min_chunk_size) {
            min_chunk_size = chunk->size;
        }
        if (chunk->size > max_chunk_size) {
            max_chunk_size = chunk->size;
        }
        chunk_size_sum += chunk->size;
    });
    fprintf(output, "]\n");
    fprintf(output, "\tMin Chunk Size: %u\n", min_chunk_size);
    fprintf(output, "\tMax Chunk Size: %u\n", max_chunk_size);
    fprintf(output, "\tAverage Chunk Size: %u\n", chunk_size_sum / chunk_count);
    fprintf(output, "\tChunks:\n");
    unsigned int chunk_index = 0;
    bytes_chunks_foreach(chunk, {
        ++chunk_index;
        fprintf(
            output,
            "\t\t%u) %c size: %u, alignment: %u\n",
            chunk_index,
            chunk->is_busy ? 'C' : 'F',
            chunk->size,
            chunk->alignment
        );
    });
    fflush(output);
}
