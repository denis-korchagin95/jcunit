#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "headers/cache.h"
#include "headers/compiler.h"
#include "headers/string.h"
#include "headers/allocator.h"
#include "headers/source.h"
#include "headers/util.h"


static struct memory_blob_pool cache_pool;
static pid_t cache_owner_pid;


void cache_init(void)
{
    memory_blob_pool_init(&cache_pool, DEFAULT_MEMORY_BLOB_SIZE, DEFAULT_MEMORY_BLOB_ALIGNMENT);
}

void cache_destroy(void)
{
    memory_blob_pool_free(&cache_pool, false);
}

void cache_set_owner_pid(void)
{
    cache_owner_pid = getpid();
}

int cache_is_owner(void)
{
    return getpid() == cache_owner_pid;
}

void * cache_alloc(size_t size)
{
    return memory_blob_pool_alloc(&cache_pool, size);
}


/* --- I/O helpers --- */

static int write_uint16(FILE * f, uint16_t v)
{
    return fwrite(&v, sizeof(v), 1, f) == 1;
}

static int write_uint32(FILE * f, uint32_t v)
{
    return fwrite(&v, sizeof(v), 1, f) == 1;
}

static int write_int64(FILE * f, int64_t v)
{
    return fwrite(&v, sizeof(v), 1, f) == 1;
}

static int read_uint16(FILE * f, uint16_t * v)
{
    return fread(v, sizeof(*v), 1, f) == 1;
}

static int read_uint32(FILE * f, uint32_t * v)
{
    return fread(v, sizeof(*v), 1, f) == 1;
}

static int read_int64(FILE * f, int64_t * v)
{
    return fread(v, sizeof(*v), 1, f) == 1;
}


/* --- Buffer-based deserialization helpers --- */

struct read_buffer {
    const unsigned char * data;
    uint32_t len;
    uint32_t pos;
};

static int buf_read(struct read_buffer * buf, void * dest, uint32_t size)
{
    if (buf->pos + size > buf->len) return 0;
    memcpy(dest, buf->data + buf->pos, size);
    buf->pos += size;
    return 1;
}

static int buf_read_uint32(struct read_buffer * buf, uint32_t * v)
{
    return buf_read(buf, v, sizeof(*v));
}

static struct string * buf_read_string_field(struct read_buffer * buf)
{
    uint32_t len;
    if (!buf_read_uint32(buf, &len)) return NULL;
    if (len == 0xFFFFFFFF) return NULL;
    if (len == 0) {
        return make_string("", 0);
    }
    if (buf->pos + len > buf->len) return NULL;
    {
        struct string * s = make_string((const char *)(buf->data + buf->pos), len);
        buf->pos += len;
        return s;
    }
}


/* --- Serialization (FILE-based, for cache_save) --- */

static int write_string_field(FILE * f, struct string * s)
{
    if (s == NULL) {
        return write_uint32(f, 0xFFFFFFFF);
    }
    if (!write_uint32(f, s->len)) return 0;
    if (s->len > 0) {
        if (fwrite(s->value, 1, s->len, f) != s->len) return 0;
    }
    return 1;
}

static int serialize_test_suite(FILE * f, struct test_suite * suite)
{
    uint32_t name_len;
    unsigned int i;

    name_len = (uint32_t)strlen(suite->name);
    if (!write_uint32(f, name_len)) return 0;
    if (fwrite(suite->name, 1, name_len, f) != name_len) return 0;

    if (!write_uint32(f, suite->tests_count)) return 0;

    for (i = 0; i < suite->tests_count; ++i) {
        struct abstract_test * test = suite->tests[i];

        if (!write_uint32(f, test->kind)) return 0;
        if (!write_uint32(f, test->flags)) return 0;
        if (!write_string_field(f, test->name)) return 0;

        if (test->kind == TEST_KIND_PROGRAM_RUNNER) {
            struct program_runner_test * prt = (struct program_runner_test *)test;
            if (!write_string_field(f, prt->given_filename)) return 0;
            if (!write_string_field(f, prt->given_file_content)) return 0;
            if (!write_string_field(f, prt->program_path)) return 0;
            if (!write_string_field(f, prt->program_args)) return 0;
            if (!write_string_field(f, prt->expected_stdout)) return 0;
            if (!write_string_field(f, prt->expected_stderr)) return 0;
        }
    }
    return 1;
}


/* --- Deserialization (buffer-based, for cache_lookup) --- */

static struct test_suite * deserialize_test_suite(
    struct read_buffer * buf,
    struct source * source
) {
    uint32_t name_len, tests_count, i;
    const char * name;
    struct test_suite * suite;

    if (!buf_read_uint32(buf, &name_len)) return NULL;
    if (buf->pos + name_len > buf->len) return NULL;
    {
        char * name_buf = memory_blob_pool_alloc(&memory_pool, name_len + 1);
        memcpy(name_buf, buf->data + buf->pos, name_len);
        name_buf[name_len] = '\0';
        buf->pos += name_len;
        name = name_buf;
    }

    if (!buf_read_uint32(buf, &tests_count)) return NULL;

    suite = memory_blob_pool_alloc_zeroed(&memory_pool, sizeof(struct test_suite));
    suite->source = source;
    suite->name = name;
    suite->tests_count = tests_count;
    suite->tests = (struct abstract_test **)memory_blob_pool_alloc(
        &memory_pool, tests_count * sizeof(void *)
    );

    for (i = 0; i < tests_count; ++i) {
        uint32_t kind, flags;

        if (!buf_read_uint32(buf, &kind)) return NULL;
        if (!buf_read_uint32(buf, &flags)) return NULL;

        if (kind == TEST_KIND_PROGRAM_RUNNER) {
            struct program_runner_test * prt = memory_blob_pool_alloc_zeroed(
                &memory_pool, sizeof(struct program_runner_test)
            );
            prt->base.kind = kind;
            prt->base.flags = flags;
            prt->base.test_suite = suite;
            prt->base.name = buf_read_string_field(buf);
            string_protect(prt->base.name);

            prt->given_filename = buf_read_string_field(buf);
            prt->given_file_content = buf_read_string_field(buf);
            prt->program_path = buf_read_string_field(buf);
            prt->program_args = buf_read_string_field(buf);
            prt->expected_stdout = buf_read_string_field(buf);
            prt->expected_stderr = buf_read_string_field(buf);

            string_protect(prt->given_filename);
            string_protect(prt->given_file_content);
            string_protect(prt->program_path);
            string_protect(prt->program_args);
            string_protect(prt->expected_stdout);
            string_protect(prt->expected_stderr);

            suite->tests[i] = (struct abstract_test *)prt;
        } else {
            return NULL;
        }
    }
    return suite;
}


/* --- Public API --- */

struct cache_store * cache_load(const char * path)
{
    FILE * f;
    char magic[CACHE_MAGIC_SIZE];
    uint32_t version, entry_count, i;
    struct cache_store * store;

    store = memory_blob_pool_alloc_zeroed(&cache_pool, sizeof(struct cache_store));

    f = fopen(path, "rb");
    if (f == NULL) {
        return store;
    }

    if (fread(magic, 1, CACHE_MAGIC_SIZE, f) != CACHE_MAGIC_SIZE
        || memcmp(magic, CACHE_MAGIC, CACHE_MAGIC_SIZE) != 0
        || !read_uint32(f, &version) || version != CACHE_VERSION
        || !read_uint32(f, &entry_count)) {
        fclose(f);
        return store;
    }
    if (entry_count > CACHE_MAX_ENTRIES) {
        entry_count = CACHE_MAX_ENTRIES;
    }

    for (i = 0; i < entry_count; ++i) {
        uint16_t path_len;
        int64_t mtime_val, size_val;
        uint32_t data_len;
        char * entry_path;
        struct cache_entry * entry;

        if (!read_uint16(f, &path_len)) break;
        entry_path = memory_blob_pool_alloc(&cache_pool, path_len + 1);
        if (fread(entry_path, 1, path_len, f) != path_len) {
            break;
        }
        entry_path[path_len] = '\0';

        if (!read_int64(f, &mtime_val)) break;
        if (!read_int64(f, &size_val)) break;
        if (!read_uint32(f, &data_len)) break;

        entry = &store->entries[store->entry_count];
        entry->path = entry_path;
        entry->mtime = (time_t)mtime_val;
        entry->file_size = (off_t)size_val;
        entry->suite = NULL;

        /* read raw serialized data */
        entry->data = memory_blob_pool_alloc(&cache_pool, data_len);
        entry->data_len = data_len;
        if (fread(entry->data, 1, data_len, f) != data_len) {
            break;
        }

        store->entry_count++;
    }

    fclose(f);
    return store;
}


struct test_suite * cache_lookup(struct cache_store * store, struct source * source)
{
    unsigned int i;
    struct stat st;

    if (store == NULL || source == NULL) return NULL;
    if (stat(source->filename, &st) != 0) return NULL;

    for (i = 0; i < store->entry_count; ++i) {
        struct cache_entry * entry = &store->entries[i];
        if (strcmp(entry->path, source->filename) == 0) {
            if (entry->mtime == st.st_mtime && entry->file_size == st.st_size) {
                /* deserialize on first hit */
                if (entry->suite == NULL && entry->data != NULL) {
                    struct read_buffer buf;
                    buf.data = entry->data;
                    buf.len = entry->data_len;
                    buf.pos = 0;
                    entry->suite = deserialize_test_suite(&buf, source);
                    if (entry->suite == NULL) {
                        /* corrupted — treat as miss */
                        return NULL;
                    }
                }
                return entry->suite;
            }
            return NULL;
        }
    }
    return NULL;
}


void cache_update(struct cache_store * store, const char * source_path, struct test_suite * suite)
{
    unsigned int i;
    struct stat st;
    struct cache_entry * entry;

    if (store == NULL) return;
    if (stat(source_path, &st) != 0) return;

    for (i = 0; i < store->entry_count; ++i) {
        entry = &store->entries[i];
        if (strcmp(entry->path, source_path) == 0) {
            entry->mtime = st.st_mtime;
            entry->file_size = st.st_size;
            entry->suite = suite;
            entry->data = NULL;
            entry->data_len = 0;
            return;
        }
    }

    if (store->entry_count >= CACHE_MAX_ENTRIES) return;
    {
        size_t len = strlen(source_path);
        char * path_copy = memory_blob_pool_alloc(&cache_pool, len + 1);
        memcpy(path_copy, source_path, len + 1);
        entry = &store->entries[store->entry_count];
        entry->path = path_copy;
    }
    entry->mtime = st.st_mtime;
    entry->file_size = st.st_size;
    entry->suite = suite;
    entry->data = NULL;
    entry->data_len = 0;
    store->entry_count++;
}


void cache_save(struct cache_store * store, const char * path)
{
    FILE * f;
    uint32_t version, i;

    if (store == NULL) return;

    f = fopen(path, "wb");
    if (f == NULL) return;

    fwrite(CACHE_MAGIC, 1, CACHE_MAGIC_SIZE, f);
    version = CACHE_VERSION;
    write_uint32(f, version);
    write_uint32(f, store->entry_count);

    for (i = 0; i < store->entry_count; ++i) {
        struct cache_entry * entry = &store->entries[i];
        uint16_t path_len = (uint16_t)strlen(entry->path);
        long data_len_pos, data_start, data_end;
        uint32_t data_len;

        write_uint16(f, path_len);
        fwrite(entry->path, 1, path_len, f);
        write_int64(f, (int64_t)entry->mtime);
        write_int64(f, (int64_t)entry->file_size);

        if (entry->suite != NULL) {
            /* serialize live suite */
            data_len_pos = ftell(f);
            write_uint32(f, 0);

            data_start = ftell(f);
            serialize_test_suite(f, entry->suite);
            data_end = ftell(f);

            data_len = (uint32_t)(data_end - data_start);
            fseek(f, data_len_pos, SEEK_SET);
            write_uint32(f, data_len);
            fseek(f, data_end, SEEK_SET);
        } else if (entry->data != NULL) {
            /* write raw data as-is */
            write_uint32(f, entry->data_len);
            fwrite(entry->data, 1, entry->data_len, f);
        } else {
            write_uint32(f, (uint32_t)0);
        }
    }

    fclose(f);
}
