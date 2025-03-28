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

- `arena_init()`: Initialize an arena with a starting size
- `arena_alloc()`: Allocate memory within the arena
- `arena_dump()`: Print memory region information
- `arena_destroy()`: Free all memory associated with the arena
- `arena_realloc()`:  Resize a previously allocated memory block within the arena
- `arena_reset()`:  Reset the arena, freeing all allocations without deallocating the regions.

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
