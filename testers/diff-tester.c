#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/allocator.h"
#include "../headers/diff.h"
#include "../headers/errors.h"


#define MAX_INPUT_SIZE (8192)

static char input_buffer[MAX_INPUT_SIZE];

static void cleanup(void)
{
    memory_blob_pool_destroy_pools();
}

int main(int argc, char * argv[])
{
    FILE * file;
    size_t bytes_read;
    char * separator;
    char * expected_start;
    unsigned int expected_len;
    char * actual_start;
    unsigned int actual_len;
    struct string * expected_str;
    struct string * actual_str;

    if (argc <= 1) {
        jcunit_fatal_error("No specified args!");
    }

    if (atexit(cleanup) != 0) {
        jcunit_fatal_error("Can't register atexit handler!");
    }

    memory_blob_pool_init_pools();

    file = fopen(argv[1], "r");
    if (file == NULL) {
        jcunit_fatal_error("Can't open file: %s", argv[1]);
    }

    bytes_read = fread(input_buffer, 1, MAX_INPUT_SIZE - 1, file);
    input_buffer[bytes_read] = '\0';
    fclose(file);

    separator = strstr(input_buffer, "\n---\n");
    if (separator == NULL) {
        jcunit_fatal_error("Missing '---' separator in input file!");
    }

    expected_start = input_buffer;
    expected_len = (unsigned int)(separator - input_buffer);

    actual_start = separator + 5;
    actual_len = (unsigned int)(bytes_read - (actual_start - input_buffer));

    if (actual_len > 0 && actual_start[actual_len - 1] == '\n') {
        --actual_len;
    }

    expected_str = make_string(expected_start, expected_len);
    actual_str = make_string(actual_start, actual_len);

    print_diff(stdout, expected_str, actual_str);

    return 0;
}
