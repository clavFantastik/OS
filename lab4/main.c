#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "allocator.h"

Allocator* fallback_allocator_create(void* memory, size_t size) {
    return NULL;
}

void fallback_allocator_destroy(Allocator* allocator) {
    ;
}

void* fallback_allocator_alloc(Allocator* allocator, size_t size) {
    void* memory = malloc(size);
    return memory;
}

void fallback_allocator_free(Allocator* allocator, void* memory) { 
    free(memory);
}

char *get_string(char * s, int *len) {
    *len = 0;
    int capacity = 1; 

    char c = getchar();

    while (c != '\n') {
        s[(*len)++] = c; 

        c = getchar();         
    }

    s[*len] = '\0'; // завершаем строку символом конца строки

    return s;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <allocator_library> <memory_size>\n", argv[0]);
        return EXIT_FAILURE;
    }

    void *library = dlopen(argv[1], RTLD_LOCAL | RTLD_NOW);
    if (!library) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    Allocator* (*allocator_create)(void*, size_t) = dlsym(library, "allocator_create");
    void (*allocator_destroy)(Allocator*) = dlsym(library, "allocator_destroy");
    void* (*allocator_alloc)(Allocator*, size_t) = dlsym(library, "allocator_alloc");
    void (*allocator_free)(Allocator*, void*) = dlsym(library, "allocator_free");

    if (!allocator_create || !allocator_destroy || !allocator_alloc || !allocator_free) {
            fprintf(stderr, "Error loading functions from the library. Using fallback.\n");
            allocator_create = fallback_allocator_create;
            allocator_destroy = fallback_allocator_destroy;
            allocator_alloc = fallback_allocator_alloc;
            allocator_free = fallback_allocator_free;
    }

    // printf("До ммапа\n");
    size_t memory_size = (size_t)atoi(argv[2]);
    void* memory = mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    Allocator* allocator = allocator_create(memory, memory_size);

    // Пример использования аллокатора
    int size;

    // printf("Первый аллок\n");

    char* ptr1 = allocator_alloc(allocator, sizeof(char) * 100);
    get_string(ptr1, &size); 
    printf("%s\n", ptr1);
    allocator_free(allocator, ptr1);

    char* ptr2 = allocator_alloc(allocator, 200);
    get_string(ptr2, &size); 
    printf("%s\n", ptr2);
    allocator_free(allocator, ptr2);


    char* ptr3 = allocator_alloc(allocator, 50);
    get_string(ptr3, &size); 
    printf("%s\n", ptr3);
    allocator_free(allocator, ptr3);

    
    // Освобождение аллокатора
    allocator_destroy(allocator);
    munmap(memory, memory_size);
    dlclose(library);
    return EXIT_SUCCESS;
} //gcc -o main main.c -ldl