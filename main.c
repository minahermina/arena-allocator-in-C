#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define ARENA_ALLOCATOR_IMPLEMENTATION
#include "arena_allocator.h"

#define ALLOC_SIZE 10000
int main(){
    Arena arena = {0};
    arena_init(&arena, ALLOC_SIZE);


    char* arr = (char*)arena_alloc(&arena, 100);
    char* arr1 = (char*)arena_alloc(&arena, 1000);
    char* arr2 = (char*)arena_alloc(&arena, 2000);
    char* arr3 = (char*)arena_alloc(&arena, 3000);
    char* arr4 = (char*)arena_alloc(&arena, 10000);

    assert(arr4 != NULL);

    arena_destroy(&arena);

    return 0;
}
