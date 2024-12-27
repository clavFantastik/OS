#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>

// NOTE: MSVC compiler does not export symbols unless annotated
#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

typedef struct Block {
    struct Block *next;
    size_t size;
}Block;

typedef struct Allocator {
    Block *head;
    void *memory;
    size_t size;
    
}Allocator;

EXPORT Allocator* allocator_create(void *const memory, const size_t size) {
    Allocator *allocator = (Allocator *) mmap(NULL, sizeof(Allocator), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    allocator->memory = memory;
    allocator->size = size;
    allocator->head = (Block *) allocator->memory;

    allocator->head->size = size - sizeof(Block);
    allocator->head->next = NULL;

    return allocator;
}

EXPORT void allocator_destroy(Allocator *const allocator) {
    munmap(allocator, sizeof(Allocator));
}

EXPORT void* allocator_alloc(Allocator *const allocator, const size_t size) { //best fit
    Block *current = allocator->head;
    Block *pred_block = current;
    Block *found_pred_block = current;
    Block *found = NULL; 
    Block *new_block;
    size_t found_block_size = ULONG_MAX;

    while(current) { // ищем свободный блок с best fit
        if(current->size >= (size + sizeof(Block)) && current->size < found_block_size) {
            found = current;
            found_block_size = current->size;
            found_pred_block = pred_block;
        }
        pred_block = current;
        current = current->next;
    }

    new_block = (Block *) (((char *)found) + sizeof(Block) + size); //оставшийся свободным кусок памяти
    new_block->size = found->size - (sizeof(Block) + size);
    new_block->next = found->next;

    if(found_pred_block == allocator->head) {
        allocator->head = new_block;
    }
    else {
        found_pred_block->next = new_block; // нужно только это на самом деле
    }

    found->next = allocator->head;
    found->size = size;

    found = (void *)(((char *)found) + sizeof(Block));

    return found;
}

EXPORT Block* merge_block_with_the_next_one(Allocator *allocator, Block* header) { //if posible
    Block *next_one = (Block *)( ((char *)header) + sizeof(Block) + header->size);
    if(next_one == header->next) {
        header->next = next_one->next;
        header->size += next_one->size + sizeof(Block);
    }
    return header;
}

EXPORT void allocator_free(Allocator *const allocator, void *const memory) {
    Block *header = (Block *) (((char *)memory) - sizeof(Block));
    Block *current = allocator->head;
    Block *prev = NULL;
    int flag = 1, header_before_head = 0;
    

    if(header < allocator->head) { // освобождаем блок до начала списка
        header->next = allocator->head;
        allocator->head = merge_block_with_the_next_one(allocator, header); // проверка на возможность мержа исам мерж
        header_before_head = 1;
        return;
    }

    if(!(current->next)) { // свободный блок единственный
        if(header > current) { // очищаемый блок после единственного свободного
            current->next = header;
            header->next = NULL;
            current = merge_block_with_the_next_one(allocator, current);
        }
        else { // очищаемый блок до единственного свободного
            header->next = current;
            allocator->head = merge_block_with_the_next_one(allocator, header);
        }
        return;
    }

    while(current && header > current) { // поиск таких элементов, чтобы prev->curr... header ...prev->current
        prev = current; // вставить после prev
        current = current->next;
    }

    if(!header_before_head) { // общий случай вставки между prev и current
        prev->next = header;
        header->next = current;
        header = merge_block_with_the_next_one(allocator, header);
        prev = merge_block_with_the_next_one(allocator, prev);
    }
} // gcc -shared -o liballocator.so allocator_fixed.c -fPIC -lm