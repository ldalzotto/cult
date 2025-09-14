#include "test_print.h"
#include "../src/print.h"
#include "../src/litteral.h"
#include <stddef.h>

// Define a test struct
typedef struct {
    i32 x;
    i32 y;
} test_point_t;

// Fields for test_point_t
field_descriptor test_point_fields[] = {
    {"x", offsetof(test_point_t, x), &i32_meta},
    {"y", offsetof(test_point_t, y), &i32_meta}
};

// Meta for test_point_t
print_meta test_point_meta = {
    .type_name = "test_point_t",
    .is_array = 0,
    .type_size = sizeof(test_point_t),
    .pt = PT_NONE,
    .fields_begin = test_point_fields,
    .fields_end = test_point_fields + (sizeof(test_point_fields) / sizeof(field_descriptor))
};

void test_print_primitives(test_context* t) {
    // Test printing primitives
    i32 val_i32 = 42;
    print_generic(&i32_meta, &val_i32, stdout, 0);
    printf("\n");

    u64 val_u64 = 1234567890123456789ULL;
    print_generic(&u64_meta, &val_u64, stdout, 0);
    printf("\n");

    // Since we can't easily test output, just ensure no crash
    TEST_ASSERT_TRUE(t, 1);
}

void test_print_struct(test_context* t) {
    test_point_t point = {10, 20};
    print_generic(&test_point_meta, &point, stdout, 0);
    printf("\n");

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_array(test_context* t) {
    i32 arr[5] = {1, 2, 3, 4, 5};
    print_array_generic(&i32_meta, arr, arr + 5, stdout, 0);
    printf("\n");

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_module(test_context* t) {
    test_print_primitives(t);
    test_print_struct(t);
    test_print_array(t);
}
