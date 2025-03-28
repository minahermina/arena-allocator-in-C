#define ARENA_ALLOCATOR_IMPLEMENTATION
#include "arena_allocator.h"
#include <stdio.h>

int main() {
    // Initialize the arena
    Arena my_arena = {0};
    arena_init(&my_arena, ARENA_REGION_DEFAULT_CAPACITY);

    // Allocate memory using the arena
    int *numbers = (int*)arena_alloc(&my_arena, 10 * sizeof(int));
    char *string = (char*)arena_alloc(&my_arena, 100 * sizeof(char));

    if (!numbers || !string) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // Use the allocated memory
    for (int i = 0; i < 10; i++) {
        numbers[i] = i * i;
    }
    snprintf(string, 100, "Hello from the arena allocator!");

    // Print allocated data
    printf("Numbers: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\nString: %s\n", string);

    // Dump arena state (optional)
    arena_dump(&my_arena);

    // Deallocate the arena all at once
    arena_destroy(&my_arena);
    return 0;
}
