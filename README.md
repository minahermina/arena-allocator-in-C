# Arena Allocator

## What is an Arena Allocator?

An arena allocator (also known as a region-based memory allocator) is a memory management strategy that dramatically simplifies memory allocation and deallocation. Instead of allocating and freeing memory for individual objects, an arena allocator:

- Allocates memory in large contiguous blocks (regions)
- Allows rapid memory allocation without individual `free()` calls
- Provides a single, efficient method to deallocate all memory at once
- Minimizes allocation overhead

## Key Features

- Single-header library implementation
- Automatic memory management
- Memory aligned to system pages using `mmap()`
- No individual memory tracking required

## Usage Example

```c
Arena arena = {0};
arena_init(&arena, ARENA_REGION_DEFAULT_CAPACITY);

int *numbers = (int*)arena_alloc(&arena, 10 * sizeof(int));
char *string = (char*)arena_alloc(&arena, 100 * sizeof(char));

// Optional: Inspect memory usage
arena_dump(&arena);

// Clean up all allocated memory at once
arena_destroy(&arena);
```

## Installation

1. Copy `arena_allocator.h` into your project
2. In one source file, define `ARENA_ALLOCATOR_IMPLEMENTATION` before including the header, to include the definitions of the functions :

```c
#define ARENA_ALLOCATOR_IMPLEMENTATION
#include "arena_allocator.h"
```

## Current utilities

### Core Functions
- `arena_init()`: Initialize an arena with a starting size. Must be called before using any other arena functions.
- `arena_alloc()`: Allocate memory within the arena. Returns a pointer to the allocated memory block.
- `arena_realloc()`: Resize a previously allocated memory block within the arena. Creates a new allocation and copies data from the old block.
- `arena_reset()`: Reset the arena, marking all allocations as available for reuse without deallocating the underlying regions.
- `arena_destroy()`: Free all memory associated with the arena, including all regions. The arena cannot be used after this call.

### Utility Functions
- `arena_dump()`: Print detailed information about all memory regions in the arena for debugging purposes.
- `arena_strlen()`: Calculate the length of a null-terminated string (custom implementation to avoid string.h dependency).
- `arena_memcpy()`: Copy memory from source to destination (custom implementation to avoid string.h dependency).

### Dynamic Array Macros
- `ARENA_ARR(name, type)`: Define a dynamic array type with the given name and element type.
- `arena_arr_append(arena, arr, item)`: Append an item to a dynamic array, automatically growing the array as needed.

### String Manipulation Macros
- `arena_str_append(arena, str, ch)`: Append a single character to a string, automatically growing the string buffer as needed.
- `arena_str_append_cstr(arena, str, item)`: Append a C-string to an existing string, automatically growing the buffer as needed.

### Constants and Configuration
- `ARENA_REGION_DEFAULT_CAPACITY`: Default size for new regions (defaults to 2 * page size).
- `ARENA_ARR_INIT_CAPACITY`: Initial capacity for dynamic arrays (defaults to 256).
- `ARENA_ARR(name, type)`: Macro to define typed dynamic array structures.


### Future Plans
- For detailed future plans, check the `TODOs` section in [`arena_allocator.h`](./arena_allocator.h).


## Recommended Reading

For those interested in diving deeper into arena allocation and its use cases, check out these resources:

1. **Physically Based Rendering: Memory Management**
   - Detailed exploration of arena allocation in graphics programming
   - Link: https://pbr-book.org/3ed-2018/Utilities/Memory_Management

2. **Untangling Lifetimes: The Arena Allocator**
   - An in-depth article on memory allocation techniques, specifically arena allocation.
   - Link: https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator

## License

GNU General Public License v2.0
