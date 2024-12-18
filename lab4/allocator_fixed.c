#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "allocator.h"

// NOTE: MSVC compiler does not export symbols unless annotated
#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#define MAX_SIZE 4096
#define BLOCK_SIZE_COUNT (MAX_SIZE / 8)

typedef struct Block {
    struct Block* next;
} Block;

typedef struct Allocator{
    Block* free_lists[BLOCK_SIZE_COUNT];
    void* memory;
    size_t total_size;
} Allocator;

EXPORT Allocator* allocator_create(void* memory, const size_t size) {
    Allocator* allocator = mmap(NULL, sizeof(Allocator), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (allocator == MAP_FAILED) {
        return NULL; // Ошибка при выделении памяти
    }

    allocator->memory = memory;
    allocator->total_size = size;
    memset(allocator->free_lists, 0, sizeof(allocator->free_lists));
    return allocator;
}

EXPORT void allocator_destroy(Allocator* allocator) {
    munmap(allocator, sizeof(Allocator));
}

static int block_index(size_t size) {
    int index = 0;
    size_t block_size = 8;
    while (block_size < size && index < BLOCK_SIZE_COUNT) {
        block_size *= 2;
        index++;
    }
    return index;
}

EXPORT void* allocator_alloc(Allocator* allocator, const size_t size) {
    int index = block_index(size);
    if (index >= BLOCK_SIZE_COUNT) return NULL;

    if (allocator->free_lists[index]) {
        Block* block = allocator->free_lists[index];
        allocator->free_lists[index] = block->next;
        return block;
    }

    return NULL;
}

EXPORT void allocator_free(Allocator* allocator, void* ptr, size_t size) {
    int index = block_index(size);
    if (index < BLOCK_SIZE_COUNT) {
        Block* block = (Block*)ptr;
        block->next = allocator->free_lists[index];
        allocator->free_lists[index] = block;
    }
}
