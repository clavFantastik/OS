#include "allocator.h"

// NOTE: MSVC compiler does not export symbols unless annotated
#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

typedef struct Block {
    size_t size;
    struct Block* next;
} Block;

typedef struct Allocator{
    Block* free_list;
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
    allocator->free_list = (Block*)memory;
    allocator->free_list->size = size - sizeof(Block);
    allocator->free_list->next = NULL;
    return allocator;
}

EXPORT void allocator_destroy(Allocator* allocator) {
    munmap(allocator, sizeof(Allocator));
}

EXPORT void* allocator_alloc(Allocator* allocator, const size_t size) {
    Block* current = allocator->free_list;
    Block* previous = NULL;

    size_t total_size = size + sizeof(Block);

    while (current) {
        if (current->size >= total_size) {
            if (current->size > total_size + sizeof(Block)) {
                Block* new_block = (Block*)((char*)current + total_size);
                new_block->size = current->size - total_size;
                new_block->next = current->next;
                current->size = size;
                current->next = new_block;
            } else {
                if (previous) {
                    previous->next = current->next;
                } else {
                    allocator->free_list = current->next;
                }
            }
            return (char*)current + sizeof(Block);
        }
        previous = current;
        current = current->next;
    }
    return NULL;
}

EXPORT void allocator_free(Allocator* allocator, void* ptr) {
    Block* block = (Block*)((char*)ptr - sizeof(Block));
    block->next = allocator->free_list;
    allocator->free_list = block;
}
