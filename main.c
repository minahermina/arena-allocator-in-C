#define ARENA_ALLOCATOR_IMPLEMENTATION
#include "arena_allocator.h"
#include <stdio.h>

#define NO_ELEMENTS 10

typedef struct {
    Arena *arena;
    int **numbers;
} Arg;

void* alloc_numbers1(void * ptr)
{
    Arg *arg = (Arg*) ptr;
    *(arg->numbers) = (int*) arena_alloc(arg->arena, NO_ELEMENTS * sizeof(int));

    for (int i = 0; i < NO_ELEMENTS; i++) {
        (*(arg->numbers))[i] = i;
    }

    printf("Numbers1: ");
    for (int i = 0; i < NO_ELEMENTS; i++) {
        printf("%d ", (*(arg->numbers))[i]);
    }
    printf("\n");
    return  NULL;
}

void* alloc_numbers2(void * ptr)
{
    Arg *arg = (Arg*) ptr;
    *(arg->numbers) = (int*) arena_alloc(arg->arena, NO_ELEMENTS * sizeof(int));

    for (int i = 0; i < NO_ELEMENTS; i++) {
        (*(arg->numbers))[i] = i * 2;
    }

    printf("Numbers1: ");
    for (int i = 0; i < NO_ELEMENTS; i++) {
        printf("%d ", (*(arg->numbers))[i]);
    }
    printf("\n");
    return  NULL;
}

int main() 
{
    // Initialize the arena
    Arena arena = {0};
    pthread_t thread1, thread2;
    int ret1, ret2;
    arena_init(&arena, ARENA_REGION_DEFAULT_CAPACITY);
    int *numbers1 = 0, *numbers2 = 0;

    Arg arg1 = {&arena, &numbers1};
    Arg arg2 = {&arena, &numbers2};
    printf("numbers1 before: %p\n", numbers1);

    ret1 = pthread_create(&thread1, NULL,  alloc_numbers1, (void *) &arg1);
    ret2 = pthread_create(&thread2, NULL,  alloc_numbers2, (void *) &arg2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Thread 1 returns: %d\n", ret1);
    printf("numbers1 after thread1: %p\n\n", numbers1);

    printf("numbers1 start at: %p\n", numbers1);
    printf("numbers1 ends at: %p\n\n", numbers1 + NO_ELEMENTS - 1);
    printf("numbers2 start at: %p\n", numbers2);
    printf("numbers2 ends at: %p\n\n", numbers2 + NO_ELEMENTS - 1);

    printf("Thread 2 returns: %d\n", ret2);

    // Dump arena state (optional)
    arena_dump(&arena);

    // Deallocate the arena all at once
    arena_reset(&arena);
    return 0;
}
