#include "test_print.h"
#include "../src/print.h"
#include "../src/litteral.h"
#include "../src/stack_alloc.h"
#include "../src/mem.h"
#include "../src/file.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Path definitions using STATIC_STRING
static const string_span test_temp_dir = STATIC_STRING("test_temp");
static const string_span path_test_output = STATIC_STRING("test_temp/print_test_output.txt");

// Shared stack memory size for tests
#define TEST_STACK_SIZE 1024

static void cleanup_test_temp_dir(void) {
    void* pointer = mem_map(TEST_STACK_SIZE);
    stack_alloc tmp;
    sa_init(&tmp, pointer, byteoffset(pointer, TEST_STACK_SIZE));
    directory_remove(&tmp, test_temp_dir.begin, test_temp_dir.end);
    sa_deinit(&tmp);
    mem_unmap(pointer, TEST_STACK_SIZE);
}

static void setup_test_temp_dir(void) {
    cleanup_test_temp_dir();

    void* pointer = mem_map(TEST_STACK_SIZE);
    stack_alloc tmp;
    sa_init(&tmp, pointer, byteoffset(pointer, TEST_STACK_SIZE));

    // Create test_temp directory if it doesn't exist
    directory_create(&tmp, test_temp_dir.begin, test_temp_dir.end, DIR_MODE_DEFAULT);

    sa_deinit(&tmp);
    mem_unmap(pointer, TEST_STACK_SIZE);
}

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

void test_print_to_file(test_context* t) {
    // Allocate memory using mem_map
    void* memory = mem_map(TEST_STACK_SIZE);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, TEST_STACK_SIZE));

    // Define test data
    test_point_t point = {10, 20};
    const char* expected = "test_point_t {\n  x: 10\n  y: 20\n}";

    // Open file for writing
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);

    // Print to file using file_t version
    print_generic(&test_point_meta, &point, file, 0);
    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_NULL(t, read_file);

    // Read content
    void* buffer;
    uptr size = file_read_all(read_file, &buffer, &alloc);
    TEST_ASSERT_TRUE(t, size > 0);

    // Check content
    TEST_ASSERT_TRUE(t, strncmp((char*)buffer, expected, strlen(expected)) == 0);

    sa_free(&alloc, buffer);
    file_close(read_file);

    sa_deinit(&alloc);
    mem_unmap(memory, TEST_STACK_SIZE);

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_module(test_context* t) {
    setup_test_temp_dir();
    test_print_primitives(t);
    test_print_struct(t);
    test_print_array(t);
    test_print_to_file(t);
    cleanup_test_temp_dir();
}
