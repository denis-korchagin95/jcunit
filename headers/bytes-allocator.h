#ifndef BYTES_ALLOCATOR_H
#define BYTES_ALLOCATOR_H 1

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

void * alloc_bytes(unsigned int len);
void free_bytes(void * mem);

void show_bytes_allocator_stats(FILE * output, bool show_leak_only);

#endif /* BYTES_ALLOCATOR_H */
