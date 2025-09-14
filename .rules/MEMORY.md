### MOST CRITICAL: STACK ALLOCATOR MEMORY REVENUE PATTERN

__THIS IS THE MOST IMPORTANT RULE IN THE ENTIRE PROJECT.__

## Memory Revenue Pattern Enforcement

All memory operations returning collections MUST return contiguous ranges using the following pattern:

### REQUIRED Pattern:

1. __Return the start pointer__ to the allocated range (not a wrapper struct)
2. __NO separate count returns or wrapper structures__ - let math determine boundaries
3. __Iterate linearly__ through contiguous memory

### Prohibited Alternatives:

- ❌ Returning wrapper structs with separate `count` fields
- ❌ Separate allocation for count variables
- ❌ Any abstraction that obscures the contiguous nature
- ❌ Non-linear memory layouts

### Core Principle:

Memory revenue is calculated, not explicitly stored. This eliminates redundant data structures and leverages the stack allocator's linear property directly.

### Example Implementation:

```c
// CORRECT - poll_xxx or any collection-returning function returns start pointer
foo_t* collection_begin = poll_xxx(context, &alloc);
foo_t* collection_end = alloc.cursor;
// Loop through contiguous collection without separate count variable
for (foo_t* item = collection_begin; item != collection_end; ++item) {
    // Process item
    process_item(item);
}
// Free entire collection block back to before allocation
sa_free(&alloc, collection_begin);

// INCORRECT - wrapper with explicit count (violates pattern)
// typedef struct { foo_t* items; i32 count; } collection_t; // NOT ALLOWED
```

### Why This is MOST Critical:

- __Fundamentally Exploits__ stack allocator's linear allocation property
- __Eliminates Redundancy__ - no extra count fields or structures
- __Ensures Consistency__ - all collections managed the same way
- __Mathematical Precision__ - exact boundary calculation
- __Performance Optimum__ - zero overhead abstraction
- __Memory Efficiency__ - no wasted space on metadata

__VIOLATING THIS PATTERN IS THE MOST SERIOUS CODE INFRACTION IN THE PROJECT.__ __ALL MEMORY OPERATIONS MUST FOLLOW THIS PATTERN.__

---

## Memory Management

### Allocation

- Use `stack_alloc` module exclusively for dynamic memory.
- Prohibited: `malloc`, `calloc`, `realloc`, `free`.
- Initialize with `sa_init()`, allocate with `sa_alloc()`, free with `sa_free()` or reset cursor.
- For large blocks, acquire via `mem_map()` and manage with stack allocator.

### Operations

- Use `byteoffset(ptr, offset)` for byte offsets instead of `(char*)ptr + offset`.
- Use `bytesize(from, to)` for byte sizes instead of `(char*)to - (char*)from`.
- Avoid manual `(char*)` casting for offsets/sizes.

### Example

```c
#include "stack_alloc.h"
#include "mem.h"

void* memory = mem_map(1024);
stack_alloc alloc;
sa_init(&alloc, memory, byteoffset(memory, 1024));
void* ptr = sa_alloc(&alloc, 64);
// Use ptr...
sa_free(&alloc, ptr);
alloc.cursor = alloc.begin; // Reset
sa_deinit(&alloc);
mem_unmap(memory, 1024);
```
