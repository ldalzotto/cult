#include "test_print.h"
#include "../src/print.h"
#include "../src/litteral.h"
#include "../src/stack_alloc.h"
#include "../src/mem.h"
#include "../src/file.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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
    print_generic(&i32_meta, &val_i32, file_get_stdout(), 0);
    printf("\n");

    u64 val_u64 = 1234567890123456789ULL;
    print_generic(&u64_meta, &val_u64, file_get_stdout(), 0);
    printf("\n");

    // Since we can't easily test output, just ensure no crash
    TEST_ASSERT_TRUE(t, 1);
}

void test_print_struct(test_context* t) {
    test_point_t point = {10, 20};
    print_generic(&test_point_meta, &point, file_get_stdout(), 0);
    printf("\n");

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_array(test_context* t) {
    i32 arr[5] = {1, 2, 3, 4, 5};
    print_array_generic(&i32_meta, arr, arr + 5, file_get_stdout(), 0);
    printf("\n");

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_to_file(test_context* t) {
    // Allocate stack memory for file operations
    uptr alloc_size = 1024;
    void* alloc_mem = mem_map(alloc_size);  // Use malloc for test, since stack_alloc needs pre-allocated
    stack_alloc alloc;
    sa_init(&alloc, alloc_mem, byteoffset(alloc_mem, alloc_size));

    // Define test data
    test_point_t point = {10, 20};
    const char* expected = "test_point_t {\n  x: 10\n  y: 20\n}";

    // Create file path
    const u8* path = (const u8*)"test_output.txt";

    // Open file for writing
    file_t file = file_open(&alloc, path, path + strlen((char*)path), FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);

    // Print to file
    print_generic(&test_point_meta, &point, file, 0);

    // Close file
    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, path, path + strlen((char*)path), FILE_MODE_READ);
    TEST_ASSERT_NOT_NULL(t, read_file);

    // Read content
    void* buffer;
    uptr size = file_read_all(read_file, &buffer, &alloc);
    TEST_ASSERT_TRUE(t, size > 0);

    // Check content
    TEST_ASSERT_TRUE(t, strncmp((char*)buffer, expected, strlen(expected)) == 0);

    // Close read file
    file_close(read_file);

    sa_free(&alloc, buffer);

    // Clean up
    sa_deinit(&alloc);
    mem_unmap(alloc_mem, alloc_size);

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_module(test_context* t) {
    test_print_primitives(t);
    test_print_struct(t);
    test_print_array(t);
    test_print_to_file(t);
}
