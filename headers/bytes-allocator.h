#ifndef BYTES_ALLOCATOR_H
#define BYTES_ALLOCATOR_H 1

#include <stdint.h>

void * alloc_bytes(unsigned int len);
void free_bytes(void * mem);

void show_bytes_allocator_stats(FILE * output);

#endif /* BYTES_ALLOCATOR_H */
