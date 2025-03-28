/*
A single-header region-based memory allocator in C
Copyright (C) 2025 Mina Albert Saeed <mina.albert.saeed@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
Usage Instructions:
This is a single-header library. To include only the function declarations, simply #include this file.
To include the implementation, define `ARENA_ALLOCATOR_IMPLEMENTATION` **in exactly one source file** before including this header:

    #define ARENA_ALLOCATOR_IMPLEMENTATION
    #include "arena_allocator.h"

    ==> Typical Usage Flow:
        1. Create an Arena: Initialize the arena with a starting size
            Arena arena = {0};
            arena_init(&my_arena, ARENA_REGION_DEFAULT_CAPACITY);

        2. Allocate Memory: Use arena_alloc to allocate memory
            int *numbers = (int*)arena_alloc(&my_arena, 10 * sizeof(int));
            char *string = (char*)arena_alloc(&my_arena, 100 * sizeof(char));

        3. (Optional) Inspect Arena: Use arena_dump to print regions information
            arena_dump(&my_arena);

        4. Clean Up: Destroy the arena when done
            arena_destroy(&my_arena);

    ==> NOTE:
        -- private functions uses second underscore like arena__* and should not be used by the user.
        -- These functions are used internally by the library.

*/
#ifndef ARENA
#define ARENA
#include <sys/mman.h>
#include <assert.h>


typedef struct Region Region;

struct Region{
    Region *next;
    size_t capacity;
    size_t count;
    size_t remaining;
    unsigned char *bytes;
};

typedef struct {
    Region *head;
    Region *tail;
} Arena;

#define ARENA_REGION_SIZE               (sizeof(Region))
#define ARENA_PAGE_SIZE                 (sysconf(_SC_PAGESIZE))

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY   (ARENA_PAGE_SIZE * 2)
#endif /*ARENA_REGION_DEFAULT_CAPACITY */

void arena_init(Arena *arena, size_t size);
void *arena_alloc(Arena *arena, size_t size);
void arena_dump(Arena *arena);
void arena_destroy(Arena *arena);

Region* arena__new__region(size_t capacity);
void arena__append__region(Arena *arena, size_t size);
size_t arena__align__size(size_t size);
void arena__region__dump(Region* region);
void arena__free__region(Region* region);

#endif /*ARENA*/
// #define ARENA_ALLOCATOR_IMPLEMENTATION

#ifdef ARENA_ALLOCATOR_IMPLEMENTATION

size_t
arena__align__size(size_t size)
{
    size_t size_page_aligned, page_size, region_size, size_bytes;
    region_size = ARENA_REGION_SIZE;
    page_size = sysconf(_SC_PAGESIZE);
    size_bytes = region_size + size;
    size_page_aligned = (size_bytes + page_size - 1) & ~(page_size - 1);
    return size_page_aligned;
}

/* This must be called at the beginning of the lifetime to initialize the arena*/
void
arena_init(Arena *arena, size_t size)
{
    Region * region;
    size = arena__align__size(size);
    region = arena__new__region(size);

    arena->head = region;
    arena->tail = region;
}

void*
arena_alloc(Arena *arena, size_t size)
{
    Region *curr_region;
    void *ptr = NULL;

    assert(arena != NULL);
    assert(arena->head != NULL);

    curr_region = arena->head;
    while(curr_region != NULL){
        if(size <= curr_region->remaining) {
            ptr = (void*)(curr_region->bytes + curr_region->count);
            curr_region->count      += size;
            curr_region->remaining  -= size;
            return ptr;
        }
        curr_region = curr_region->next;
    }

    // Allocate new region as no space available
    arena__append__region(arena, size);
    curr_region = arena->tail;
    ptr = (void*)(curr_region->bytes + curr_region->count);

    return ptr;
}

void
arena_dump(Arena *arena)
{
    Region *current;

    assert(arena != NULL);

    current = arena->head;
    while(current != NULL){
        arena__region__dump(current);
        current= current->next;
    }
}

void
arena_destroy(Arena *arena)
{
    Region* curr_region, *temp;
    curr_region = arena->head;
    while(curr_region != NULL){
        temp = curr_region;
        curr_region = curr_region->next;
        arena__free__region(temp);
    }
    arena->head = NULL;
    arena->tail = NULL;
}

Region*
arena__new__region(size_t size)
{
    Region *region;
    void *ptr;

    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    assert(ptr != MAP_FAILED);

    region = (Region*) ptr;
    region->next       = NULL;
    region->capacity   = size - ARENA_REGION_SIZE;
    region->remaining  = size - ARENA_REGION_SIZE;
    region->count      = 0;
    region->bytes      = ((unsigned char*)ptr) + ARENA_REGION_SIZE;

    return region;
}

void
arena__append__region(Arena *arena, size_t size)
{
    Region *region;
    size = arena__align__size(size);
    region = arena__new__region(size);
    arena->tail->next = region;
    arena->tail = region;
}

void
arena__region__dump(Region* region)
{
    assert(region != NULL);
    printf("===> Region Dump\n");
    printf("Address:    %p\n", (void*)region);
    printf("Next:       %p\n", (void*)region->next);
    printf("Capacity:   %zu bytes\n", region->capacity);
    printf("Used:       %zu bytes\n", region->count);
    printf("Free:       %zu bytes\n", region->remaining);
    printf("\n");
}

void
arena__free__region(Region* region)
{
    assert(region != NULL);
    size_t size_bytes = ARENA_REGION_SIZE + region->capacity;
    int ret = munmap(region, size_bytes);
    assert(ret == 0);
}

#endif /*ARENA_ALLOCATOR_IMPLEMENTATION*/
