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
*/
#ifndef ARENA
#define ARENA
#include<sys/mman.h>
#include<assert.h>
#include <inttypes.h>

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY (8*1024)
#endif /*ARENA_REGION_DEFAULT_CAPACITY */

typedef struct Region Region;

struct Region{
    Region *next;
    Region *prev;
    size_t capacity;
    size_t count;
    uintptr_t *words; /*Array of words*/
};

typedef struct {
    Region *head;
    Region *tail;
} Arena;

const size_t ARENA_REGION_SIZE  = sizeof(Region);

void arena_create(Arena *arena, size_t size);
void arena_dump(Arena *arena);
void *arena_alloc(Arena *arena, size_t size);
void arena_destroy(Arena *arena);
size_t arena__align__size(size_t size);

void region_dump(Region* region);
Region* arena__new__region(size_t capacity);
void arena__region__dump(Region* region);
#endif /*ARENA*/
// #define ARENA_ALLOCATOR_IMPLEMENTATION

#ifdef ARENA_ALLOCATOR_IMPLEMENTATION
/* This must be called at the beginning of the lifetime to initialize the arena*/
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
void
arena_create(Arena *arena, size_t size)
{
    Region * region;
    size = arena__align__size(size);
    region = arena__new__region(size);

    // head, tail
    arena->head = region;
    arena->tail = region;
}

void*
arena_alloc(Arena *arena, size_t size)
{

}

void arena_destroy(Arena *arena){


}

Region*
arena__new__region(size_t capacity)
{
    Region *region;
    unsigned char *ptr;


    ptr = mmap(NULL, capacity, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    assert(ptr != MAP_FAILED);

    region = (Region*) ptr;
    region->next = NULL;
    region->prev = NULL;
    region->capacity = capacity - ARENA_REGION_SIZE;
    region->count = 0;
    region->words = (uintptr_t*)((char*)ptr + ARENA_REGION_SIZE);

    return (Region*)ptr;
}

void
arena_dump(Arena *arena)
{
    Region *current;
    size_t cnt = 0;
    assert(arena != NULL);

    current = arena->head;

    printf("===> %zu\n", cnt);
    while(current != NULL){
        arena__region__dump(current);
        current= current->next;
    }
}

void
arena__region__dump(Region* region)
{
    printf("===> Region Dump\n");
    printf("Address: %p\n", (void*)region);
    printf("Prev:    %p\n", (void*)region->prev);
    printf("Next:    %p\n", (void*)region->next);
    printf("Capacity: %zu bytes\n", region->capacity);
    printf("Used:     %zu bytes\n", region->count);
    printf("Free:     %zu bytes\n", region->capacity - region->count);

    printf("First 8 words of data:\n");
    for (size_t i = 0; i < 8 && i < region->count / sizeof(uintptr_t); i++) {
        printf("  words[%zu]: 0x%" PRIxPTR "\n", i, region->words[i]);
    }
    printf("\n");
}

#endif /*ARENA_ALLOCATOR_IMPLEMENTATION*/
