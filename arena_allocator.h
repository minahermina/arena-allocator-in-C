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


    ==> TODOs for Arena Allocator:
        - [x] Dynamic array manipulation.
        - [ ] Store Region meta data out of band
        - [x] String handling and management.
        - [ ] Implement a better reallocation strategy to minimize wasted memory.
        - [ ] Improve memory alignment.
        - [ ] Implement debugging utilities for tracking memory usage.
        - [x] Implement thread safety with mutex locking
        - [ ] Add thread-local storage support for better multi-threaded performance


*/
#ifndef ARENA_ALLOCATOR
#define ARENA_ALLOCATOR
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

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
    pthread_mutex_t mutex;
} Arena;


#define ARENA_ARR(name, type) \
    typedef struct name { \
        type *items; \
        size_t size; \
        size_t capacity; \
    } name

#define ARENA_REGION_SIZE        (sizeof(Region))
#define ARENA_PAGE_SIZE          (sysconf(_SC_PAGESIZE))
#define ARENA_SIZE_ARR(arr)      (sizeof(arr) / sizeof((arr)[0]))

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY   (ARENA_PAGE_SIZE * 2)
#endif /*ARENA_REGION_DEFAULT_CAPACITY */


#ifndef ARENA_ARR_INIT_CAPACITY
#define ARENA_ARR_INIT_CAPACITY 256
#endif // ARENA_DA_INIT_CAP

/*Functions declarations*/
void arena_init(Arena *arena, size_t size);
void *arena_alloc(Arena *arena, size_t size);
void *arena_realloc(Arena *arena, void *oldptr, size_t oldsz, size_t newsz);
size_t arena_strlen(const char *str); /* this is implemented  instead of including <string.h>*/
void *arena_memcpy(void *dest, const void *src, size_t n); /* just like arena_strlen*/
void arena_dump(Arena *arena);

/* Must be used only when no other threads are using the arena*/
void arena_reset(Arena *arena);
void arena_destroy(Arena *arena);

/*Private Functions declarations*/
void *arena__alloc__unlocked(Arena *arena, size_t size);
Region* arena__new__region(size_t capacity);
void arena__append__region(Arena *arena, size_t size);
size_t arena__align__size(size_t size);
void arena__region__dump(Region* region);
void arena__free__region(Region* region);

#endif /*ARENA_ALLOCATOR*/


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
    Region *region;
    int ret;
    size = arena__align__size(size);
    region = arena__new__region(size);

    arena->head = region;
    arena->tail = region;

    /* Init the mutex */
    ret = pthread_mutex_init(&arena->mutex, NULL);
    assert(ret == 0);
}

void*
arena__alloc__unlocked(Arena *arena, size_t size)
{
    Region *curr;
    void *ptr;
    int ret;

    assert(arena != NULL);
    assert(arena->head != NULL);

    for(curr = arena->head; curr != NULL; curr = curr->next ){
        if(size <= curr->remaining) {
            ptr = (void*)(curr->bytes + curr->count);
            curr->count      += size;
            curr->remaining  -= size;

            return ptr;
        }
    }

    // Allocate new region as no space available
    arena__append__region(arena, size);
    curr = arena->tail;
    ptr = (void*)(curr->bytes + curr->count);

    return ptr;
}


void*
arena_alloc(Arena *arena, size_t size)
{
    void *ptr;
    int ret;

    assert(arena != NULL);
    assert(arena->head != NULL);

    /* Locking the mutex */
    ret = pthread_mutex_lock(&arena->mutex);
    assert(ret == 0);

    ptr = arena__alloc__unlocked(arena, size);

    /* Unlocking the mutex */
    ret = pthread_mutex_unlock(&arena->mutex);
    assert(ret == 0);

    return ptr;
}

size_t
arena_strlen(const char *str)
{
    size_t sz = 0;
    while(*str){
        sz++;
        str++;
    }
    return sz;
}

void *
arena_memcpy(void *dest, const void *src, size_t n)
{
    const char *_src = src;
    char *_dest = dest;
    for( ; n !=0; n--){
        *_dest = *_src;
        _dest++;
        _src++;
    }
    return _dest;
}

#define arena_arr_append(arena, arr, item) \
    do{ \
        if((arr)->size >= (arr)->capacity) { \
            size_t new_capacity = (arr)->capacity == 0 ? ARENA_ARR_INIT_CAPACITY : (arr)->capacity*2;  \
            (arr)->items = arena_realloc(arena, \
                                         (arr)->items, \
                                         (arr)->capacity*sizeof(*(arr)->items), \
                                         new_capacity*sizeof(*(arr)->items)); \
            (arr)->capacity = new_capacity; \
        } \
        (arr)->items[(arr)->size] = item;\
        (arr)->size++; \
    } while(0)



#define arena_str_append(a, str, ch) \
    do { \
        if ((str)->size + 2 > (str)->capacity) { /* +2 for char and null terminator */ \
            size_t new_capacity = (str)->capacity ? (str)->capacity * 2 : ARENA_ARR_INIT_CAPACITY; \
            (str)->items = arena_realloc((a), \
                                        (str)->items, \
                                        (str)->capacity * sizeof(char), \
                                        new_capacity * sizeof(char)); \
            (str)->capacity = new_capacity; \
        } \
        (str)->items[(str)->size] = (ch); \
        (str)->size++; \
        (str)->items[(str)->size] = '\0'; \
    } while(0)

#define arena_str_append_cstr(a, str, item) \
    do { \
        size_t len = arena_strlen(item); \
        size_t new_size = (str)->size + len; \
        if (new_size + 1 > (str)->capacity) { /* +1 for null terminator */ \
            size_t new_capacity = (str)->capacity ? (str)->capacity * 2 : ARENA_ARR_INIT_CAPACITY; \
            while (new_capacity < new_size + 1) new_capacity *= 2; \
            (str)->items = arena_realloc((a), \
                                        (str)->items, \
                                        (str)->capacity * sizeof(char), \
                                        new_capacity * sizeof(char)); \
            (str)->capacity = new_capacity; \
        } \
        arena_memcpy((str)->items + (str)->size, (item), len); \
        (str)->size = new_size; \
        (str)->items[(str)->size] = '\0'; \
    } while(0)

/*
    Memory in the arena allocator is managed in a linear fashion,
    meaning previously allocated blocks cannot be individually freed or reused.
    When reallocating, a new block is allocated, and the old block remains unused,
    effectively making it "orphaned." This can lead to increased memory usage over time.

    TODO: Find a better approach to handle reallocations
*/
void *
arena_realloc(Arena *arena, void *old_ptr, size_t old_size, size_t new_size)
{
    unsigned char *new_ptr;
    size_t i;
    int ret;
    assert(arena != NULL);

    if(new_size < old_size)
        return old_ptr;

    /* Locking the mutex */
    ret = pthread_mutex_lock(&arena->mutex);
    assert(ret == 0);

    new_ptr = (unsigned char*)arena__alloc__unlocked(arena, new_size);

    unsigned char * old_ptr_char = (unsigned char*)old_ptr;
    for(i = 0; i < old_size; ++i){ /*Assuming no overlap happens*/
        new_ptr[i] = old_ptr_char[i];
    }

    /* Unlocking the mutex */
    ret = pthread_mutex_unlock(&arena->mutex);
    assert(ret == 0);
    return (void*) new_ptr;
}


void
arena_dump(Arena *arena)
{
    Region *curr;
    size_t cnt = 0;

    assert(arena != NULL);

    printf("=============================\n");
    for(curr = arena->head; curr != NULL; curr = curr->next ){
        printf("===> Region %zu:\n", cnt);
        arena__region__dump(curr);
        cnt++;
    }
    printf("=============================\n");
    printf("\n");
}

void
arena_reset(Arena *arena){
    Region *curr;
    int ret;
    assert(arena != NULL);

    for(curr = arena->head; curr != NULL; curr = curr->next){
        curr->count = 0;
        curr->remaining = curr->capacity;
    }

    /* Safe to destroy - no other threads should be using it */
    ret = pthread_mutex_destroy(&arena->mutex);
    assert(ret == 0);

    /* Re-initialize the mutex for future use */
    ret = pthread_mutex_init(&arena->mutex, NULL);
    assert(ret == 0);
}

void
arena_destroy(Arena *arena)
{
    Region* curr, *temp;
    int ret;

    for(curr = arena->head; curr != NULL;){
        temp = curr;
        curr = curr->next;
        arena__free__region(temp);
    }
    arena->head = NULL;
    arena->tail = NULL;

    /* Safe to destroy - no other threads should be using it */
    ret = pthread_mutex_destroy(&arena->mutex);
    assert(ret == 0);
}

Region*
arena__new__region(size_t size)
{
    Region *region;
    void *ptr;

    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    assert(ptr != MAP_FAILED);

    region             = (Region*) ptr;
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
    if(size < (size_t)ARENA_REGION_DEFAULT_CAPACITY){
        size = ARENA_REGION_DEFAULT_CAPACITY;
    } else{
        size = arena__align__size(size);
    }
    region = arena__new__region(size);
    arena->tail->next = region;
    arena->tail = region;
}

void
arena__region__dump(Region* region)
{
    assert(region != NULL);
    printf("Address:    %p\n", (void*)region);
    printf("Starts at:  %p\n", (void*)region->bytes);
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

