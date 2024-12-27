#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

typedef struct Allocator Allocator;
typedef struct Block Block;

typedef Allocator* allocator_create_func(void* memory, const size_t size);
typedef void allocator_destroy_func(Allocator* allocator);
typedef void* allocator_alloc_func(Allocator* allocator, const size_t size);
typedef void allocator_free_func(Allocator* allocator, void* ptr, size_t size);
#endif