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

# Project Coding Rules - Type Usage

## Type Usage Enforcement

This project enforces the exclusive use of custom primitive types defined in `src/primitive.h` instead of standard C types to ensure consistent type sizes across different platforms.

### Prohibited Types
- Do NOT use `char`, `short`, `int`, `long`, `long long` (signed or unsigned variants)
- Do NOT use direct standard C integer types for any variable declarations, function parameters, or return types

### Required Types
- For 8-bit signed integers: Use `i8`
- For 16-bit signed integers: Use `i16`
- For 32-bit signed integers: Use `i32`
- For 64-bit signed integers: Use `i64`
- For pointer-sized signed integers: Use `imax`
- For 8-bit unsigned integers: Use `u8`
- For 16-bit unsigned integers: Use `u16`
- For 32-bit unsigned integers: Use `u32`
- For 64-bit unsigned integers: Use `u64`
- For pointer-sized unsigned integers: Use `uimax`

### Additional Notes
- Include `src/primitive.h` in all source files that require these types
- These types are defined with platform-agnostic size requirements (verified with static assertions), including pointer-sized types where sizeof(imax) == sizeof(uimax) == sizeof(void*)
- Always prefer the custom types over standard C integers for consistency and portability

## Memory Operations
- Use the `byteoffset` macro for calculating byte offsets instead of direct `char*` casting or arithmetic
- Example: Instead of `(char*)ptr + offset`, use `byteoffset(ptr, offset)`
- Use the `bytesize` macro for calculating byte sizes instead of direct pointer subtraction
- Example: Instead of `(char*)ptr2 - (char*)ptr1`, use `bytesize(ptr1, ptr2)`
- DO NOT use manual pointer arithmetic with `(char*)` casting for offsets or size calculations
- For pointer comparisons, use direct void* comparison if pointers are within the same memory object

### Example Usage
```c
#include "primitive.h"

// Correct usage
i32 myVariable = 42;
u16 anotherVar = 100;
i64 bigNumber = 9223372036854775807LL;

// Incorrect usage (DO NOT USE)
int standardInt = 42;
unsigned int standardUint = 100;
long standardLong = 0;
```

## Memory Allocation Enforcement

This project enforces the exclusive use of the stack_alloc module for all dynamic memory allocations to ensure consistent and controlled memory management.

### Prohibited Allocation Functions
- DO NOT use `malloc`, `calloc`, `realloc`, `free`
- DO NOT use any direct system memory allocation functions
- DO NOT use alternative allocators without explicit approval

### Required Allocation Method
- Use the stack_alloc module (`src/stack_alloc.h`) for all memory allocations
- Initialize the allocator with `sa_init()`
- Allocate memory with `sa_alloc()` (asserts if out of memory)
- Free memory with `sa_free()` (selective rollback) or reset the allocator
- Deinitialize with `sa_deinit()` to verify all memory is properly deallocated
- For large memory blocks, acquire memory via `mem_map()` from `src/mem.h` and manage it with the stack allocator
- Use `uptr` type for sizes where appropriate

### Allocation Strategy
- This is a stack-based allocator: allocations are linear and deallocation is typically by rolling back to a previous state
- Suitable for temporary allocations within a scope; not suitable for long-lived or complex object graphs
- Assert is triggered if allocation would exceed the memory block bounds

### Example Usage
```c
#include "stack_alloc.h"
#include "mem.h"

// Acquire memory block
void* memory = mem_map(1024);

// Initialize stack allocator
stack_alloc alloc;
sa_init(&alloc, memory, byteoffset(memory, 1024));

// Allocate memory
void* ptr = sa_alloc(&alloc, 64);

// Use ptr...

// Free specific allocation (rollback to ptr)
sa_free(&alloc, ptr);

// Or reset entire allocator for bulk deallocation
alloc.cursor = alloc.begin;

// Deinitialize (verifies cursor is back to begin)
sa_deinit(&alloc);

```
// Release memory block
mem_unmap(memory, 1024);

## Testing Enforcement

This project uses a custom test framework built on assertions with context structures for clean, non-global state management.

### Test Framework Structure
- All test code must be placed in the `tests/` directory
- Use `tests/test_framework.h` for all test macros and structures
- Tests must use the `test_context` structure passed as a parameter (no globals allowed)

### Test Function Conventions
- Test module functions must accept `test_context*` parameter
- Individual test functions must accept `test_context*` parameter
- Use descriptive function names with `test_` prefix
- Example: `void test_mem_map_small_block(test_context* t)`

### Test Macros
- `TEST_ASSERT_TRUE(ctx, expr)` - Assert expression is true
- `TEST_ASSERT_FALSE(ctx, expr)` - Assert expression is false
- `TEST_ASSERT_NULL(ctx, ptr)` - Assert pointer is NULL
- `TEST_ASSERT_NOT_NULL(ctx, ptr)` - Assert pointer is not NULL
- `TEST_ASSERT_EQUAL(ctx, a, b)` - Assert a equals b
- All macros automatically report file location and condition on failure

### Test Organization
- Place all tests for a module in function named `{module}_module(test_context* t)`
- Each individual test should be a separate static function
- Call individual tests from the module test function
- Test runner (`tests/all_tests.c`) creates context and calls all module tests

### Testing Best Practices
- Write tests that verify expected behavior, not implementation details
- Use meaningful test case names that describe what is being tested
- Test both normal and edge cases
- Include tests for error conditions and boundary values
- Keep tests isolated and independent

### Example Test Structure
```c
// In tests/test_example.c
#include "test_framework.h"
#include "../src/example.h"

static void test_example_normal_case(test_context* t) {
    // Test implementation
    TEST_ASSERT_TRUE(t, some_condition);
}

static void test_example_edge_case(test_context* t) {
    // Edge case test
    TEST_ASSERT_EQUAL(t, result, expected);
}

void test_example_module(test_context* t) {
    printf("Running Example Module Tests...\n");
    test_example_normal_case(t);
    test_example_edge_case(t);
}

// In tests/test_example.h
#ifndef TEST_EXAMPLE_H
#define TEST_EXAMPLE_H

#include "test_framework.h"

// Declaration of example module test function
void test_example_module(test_context* t);

#endif /* TEST_EXAMPLE_H */

// In tests/all_tests.c
#include "test_framework.h"
#include "test_example.h"
// ... additional module header includes ...

int main() {
    test_context ctx;
    test_reset_context(&ctx);

    test_example_module(&ctx);
    // ... other module tests ...

    test_report_context(&ctx);
    return ctx.failed > 0 ? 1 : 0;
}
```
