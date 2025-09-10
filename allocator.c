#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#include "headers/allocator.h"
#include "headers/util.h"


/**
 * This pool will be used for the entire lifetime of the program.
 */
struct memory_blob_pool permanent_pool = {0};

/**
 * This pool will be used for temporary objects that may be copied to the main pool if needed.
 */
struct memory_blob_pool temporary_pool = {0};


struct memory_blob * alloc_new_blob(size_t size)
{
    struct memory_blob * blob = malloc(sizeof(struct memory_blob));
    memset(blob, 0, sizeof(struct memory_blob));
    blob->memory = malloc(size);
    blob->capacity = size;
    return blob;
}

struct memory_blob_pool * memory_blob_pool_create(size_t blob_size, size_t alignment)
{
    struct memory_blob_pool * pool = malloc(sizeof(struct memory_blob_pool));
    memset(pool, 0, sizeof(struct memory_blob_pool));
    pool->blob_size = blob_size;
    pool->alignment = alignment;
    pool->blob_capacity = DEFAULT_MEMORY_BLOB_CAPACITY;
    pool->blobs = malloc(sizeof(struct memory_blob *) * pool->blob_capacity);
    return pool;
}

void memory_blob_pool_init(struct memory_blob_pool * pool, size_t blob_size, size_t alignment)
{
    assert(pool != NULL);

    memset(pool, 0, sizeof(struct memory_blob_pool));
    pool->blob_size = blob_size;
    pool->alignment = alignment;
    pool->blob_capacity = DEFAULT_MEMORY_BLOB_CAPACITY;
    pool->blobs = malloc(sizeof(struct memory_blob *) * pool->blob_capacity);
}

void * memory_blob_pool_alloc(struct memory_blob_pool * pool, size_t size)
{
    assert(pool != NULL);

    size_t aligned_size = align_up(size, pool->alignment);

    size_t i;
    for (i = 0; i < pool->blob_count; ++i) {
        struct memory_blob * blob = pool->blobs[i];
        size_t aligned_offset = align_up(blob->used, pool->alignment);

        if (aligned_offset + aligned_size <= blob->capacity) {
            void * ptr = blob->memory + aligned_offset;
            blob->used += aligned_offset + aligned_size;
            return ptr;
        }
    }

    if (pool->blob_count >= pool->blob_capacity) {
        pool->blob_capacity *= 2;
        pool->blobs = realloc(pool->blobs, sizeof(struct memory_blob *) * pool->blob_capacity);
    }

    struct memory_blob * new_blob = alloc_new_blob(pool->blob_size);
    pool->blobs[pool->blob_count++] = new_blob;

    size_t aligned_offset = align_up(new_blob->used, pool->alignment);
    void * ptr = new_blob->memory + aligned_offset;
    new_blob->used += aligned_offset + aligned_size;

    return ptr;
}

void memory_blob_pool_free(struct memory_blob_pool * pool, bool free_pool)
{
    assert(pool != NULL);

    size_t i;
    for (i = 0; i < pool->blob_count; ++i) {
        free(pool->blobs[i]->memory);
        free(pool->blobs[i]);
    }

    free(pool->blobs);

    if (free_pool)
        free(pool);
}

void memory_blob_pool_init_pools(void)
{
    memory_blob_pool_init(&permanent_pool, DEFAULT_MEMORY_BLOB_SIZE * 2, DEFAULT_MEMORY_BLOB_ALIGNMENT);
    memory_blob_pool_init(&temporary_pool, DEFAULT_MEMORY_BLOB_SIZE, DEFAULT_MEMORY_BLOB_ALIGNMENT);
}

void memory_blob_pool_destroy_pools(void)
{
    memory_blob_pool_free(&permanent_pool, false);
    memory_blob_pool_free(&temporary_pool, false);
}
