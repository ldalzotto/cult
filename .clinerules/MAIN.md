# MOST CRITICAL: STACK ALLOCATOR MEMORY REVENUE PATTERN

**THIS IS THE MOST IMPORTANT RULE IN THE ENTIRE PROJECT.**

## Memory Revenue Pattern Enforcement

All memory operations returning collections MUST return contiguous ranges using the following pattern:

### REQUIRED Pattern:
1. **Return the start pointer** to the allocated range (not a wrapper struct)
2. **NO separate count returns or wrapper structures** - let math determine boundaries
3. **Iterate linearly** through contiguous memory

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
- **Fundamentally Exploits** stack allocator's linear allocation property
- **Eliminates Redundancy** - no extra count fields or structures
- **Ensures Consistency** - all collections managed the same way
- **Mathematical Precision** - exact boundary calculation
- **Performance Optimum** - zero overhead abstraction
- **Memory Efficiency** - no wasted space on metadata

**VIOLATING THIS PATTERN IS THE MOST SERIOUS CODE INFRACTION IN THE PROJECT.**
**ALL MEMORY OPERATIONS MUST FOLLOW THIS PATTERN.**

---

## Type Usage Enforcement

Use custom primitive types from `src/primitive.h` instead of standard C types for consistency and portability.

### Prohibited Types
- Do NOT use `char`, `short`, `int`, `long`, `long long` (signed or unsigned variants)

### Required Types
- `i8` (8-bit signed), `u8` (8-bit unsigned)
- `i16` (16-bit signed), `u16` (16-bit unsigned)
- `i32` (32-bit signed), `u32` (32-bit unsigned)
- `i64` (64-bit signed), `u64` (64-bit unsigned)
- `iptr` (pointer-sized signed), `uptr` (pointer-sized unsigned)

Include `src/primitive.h` in all source files.

### Example Usage
```c
#include "primitive.h"

// Correct
i32 myVariable = 42;
u16 anotherVar = 100;

// Incorrect (DO NOT USE)
int standardInt = 42;
unsigned int standardUint = 100;
```

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

---

## Testing Enforcement

Use custom test framework in `tests/` directory.

### Conventions
- Test functions accept `test_context*` parameter.
- Use macros: `TEST_ASSERT_TRUE()`, `TEST_ASSERT_FALSE()`, `TEST_ASSERT_NULL()`, `TEST_ASSERT_NOT_NULL()`, `TEST_ASSERT_EQUAL()`.
- Organize tests in `{module}_module(test_context* t)` functions.
- Call from `tests/all_tests.c`.

### Example
```c
#include "test_framework.h"

static void test_example(test_context* t) {
    TEST_ASSERT_TRUE(t, condition);
}

void example_module(test_context* t) {
    test_example(t);
}
```
