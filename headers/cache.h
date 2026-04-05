#ifndef JCUNIT_CACHE_H
#define JCUNIT_CACHE_H 1

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#define CACHE_MAGIC "JCUNIT\0"
#define CACHE_MAGIC_SIZE 8
#define CACHE_VERSION 5
#define CACHE_FILENAME ".jcunit_cache"
#define CACHE_MAX_ENTRIES 1024

struct test_suite;
struct source;

struct cache_entry {
    const char * path;
    time_t mtime;
    off_t file_size;
    uint32_t content_hash;
    struct test_suite * suite;
    unsigned char * data;
    uint32_t data_len;
};

struct cache_store {
    struct cache_entry entries[CACHE_MAX_ENTRIES];
    unsigned int entry_count;
};

void cache_init(void);
void cache_destroy(void);
void cache_set_owner_pid(void);
int cache_is_owner(void);
void * cache_alloc(size_t size);
struct cache_store * cache_load(const char * path);
struct test_suite * cache_lookup(struct cache_store * store, struct source * source);
void cache_update(struct cache_store * store, const char * source_path, struct test_suite * suite);
void cache_save(struct cache_store * store, const char * path);

#endif /* JCUNIT_CACHE_H */
