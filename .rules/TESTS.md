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
