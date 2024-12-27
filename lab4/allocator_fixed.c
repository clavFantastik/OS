#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include "allocator.h"

// NOTE: MSVC compiler does not export symbols unless annotated
#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

typedef struct Block {
    struct Block *next;
    int index;
}Block;

typedef struct Allocator {
    Block **list;
    size_t size;
}Allocator;

EXPORT Allocator* allocator_create(void *const memory, const size_t size) {
    Allocator *allocator = (Allocator *) mmap(NULL, sizeof(Allocator), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // проверка на size
    // лист будет содержать 6 указателей на списки блоков, размеры каждого блока: 2^(i + 5)
    size_t multiplier = size / 3840;
    // количество блоков каждого размера: multiplier * (7 - i) ... 3840 = 6 * 32 + 5 * 64 + 4 * 128 + 3 * 256 + 2 * 512 + 1 * 1024
    allocator->size = size;
    allocator->list = (Block **) memory;
    Block* current_block = (Block *)(((char*)memory) + sizeof(Block *) * 6); // первый блок вообще из всех
    Block* next_block;
    Block* prev_block;

    for(int i = 0; i < 6; i++) {
        allocator->list[i] = current_block;
        current_block->index = i;
        size_t block_size = pow(2, i + 5);
        for(int j = 0; j < (6 - i) * multiplier; j++) {
            prev_block = current_block;
            current_block->next = (Block *)(((char *)current_block) + block_size/* + sizeof(Block)*/);
            current_block = current_block->next;
            current_block->index = i;
            if (j == ((6 - i) * multiplier - 1)) current_block->next = NULL;
        }
        prev_block->next = NULL; // последний блок в списке, ссылается на NULL
    }


    return allocator;
}

EXPORT void allocator_destroy(Allocator *const allocator) {
    munmap(allocator->list, allocator->size);
    munmap(allocator, sizeof(Allocator));
    return;
}

EXPORT void* allocator_alloc(Allocator *const allocator, const size_t size) {
    if(size > 1008 || size <= 0 || !allocator) return NULL;
    size_t size_ = size + sizeof(Block);
    int index = (fmod(log2(size_), 1.0) < __DBL_EPSILON__) ? ((int)log2(size_)) : ((int)log2(size_)) + 1;     
    index -= 5;
    Block *head = allocator->list[index]; // выделяем head

    if(!head) return NULL;
    Block *current = head->next;

    allocator->list[index] = current;
    head->next = current;
    printf("Allocated block adress %p\n", head); //Allocated block adress 0x7ffff7fb9630
    head = (Block *)((char *)head) + sizeof(Block);
    return head;
}

EXPORT void allocator_free(Allocator *const allocator, void *const memory) {
    Block* head = (Block *)((char *)memory) - sizeof(Block); //((Block)memory) -1
    Block* current = allocator->list[head->index];
    
    printf("1st position: %p, freeing: %p\n", allocator->list[head->index], head);
    allocator->list[head->index] = head;
    printf("New 1st position: %p, second one: %p\n", allocator->list[head->index], allocator->list[head->index]->next);

} // gcc -shared -o liballocator.so allocator_fixed.c -fPIC -lm