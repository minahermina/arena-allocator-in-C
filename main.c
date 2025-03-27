#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define ARENA_ALLOCATOR_IMPLEMENTATION
#include "arena_allocator.h"

#define ALLOC_SIZE 1000
int main(){
    Arena arena = {0};
    arena_create(&arena, ALLOC_SIZE);
    arena_dump(&arena);
    char* arr = arena_alloc(&arena, 100);

    arena_destroy(&arena);
    return 0;
}
