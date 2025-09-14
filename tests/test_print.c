#include "test_print.h"
#include "test_temp_dir.h"
#include "../src/print.h"
#include "../src/litteral.h"
#include "../src/stack_alloc.h"
#include "../src/mem.h"
#include "../src/file.h"
#include <stddef.h>
#include <string.h>

// Path definitions using STATIC_STRING
static const string_span path_test_output = STATIC_STRING("test_temp/print_test_output.txt");

// Define test structs
typedef struct {
    i32 x;
    i32 y;
} test_point_t;

typedef struct {
    test_point_t pos;
    i32 id;
} complex_t;

// Fields for test_point_t
field_descriptor test_point_fields[] = {
    {STATIC_STRING("x"), offsetof(test_point_t, x), &i32_meta},
    {STATIC_STRING("y"), offsetof(test_point_t, y), &i32_meta}
};

// Meta for test_point_t
print_meta test_point_meta = {
    .type_name = STATIC_STRING("test_point_t"),
    .is_array = 0,
    .type_size = sizeof(test_point_t),
    .pt = PT_NONE,
    .fields_begin = test_point_fields,
    .fields_end = test_point_fields + (sizeof(test_point_fields) / sizeof(field_descriptor))
};

// Fields for complex_t
field_descriptor complex_fields[] = {
    {STATIC_STRING("pos"), offsetof(complex_t, pos), &test_point_meta},
    {STATIC_STRING("id"), offsetof(complex_t, id), &i32_meta}
};

// Meta for complex_t
print_meta complex_meta = {
    .type_name = STATIC_STRING("complex_t"),
    .is_array = 0,
    .type_size = sizeof(complex_t),
    .pt = PT_NONE,
    .fields_begin = complex_fields,
    .fields_end = complex_fields + (sizeof(complex_fields) / sizeof(field_descriptor))
};

void test_print_primitives(test_context* t) {
    // Test printing primitives
    i32 val_i32 = 42;
    print_generic(&i32_meta, &val_i32, file_stdout(), 0);
    print_string(file_stdout(), "\n");

    u64 val_u64 = 1234567890123456789ULL;
    print_generic(&u64_meta, &val_u64, file_stdout(), 0);
    print_string(file_stdout(), "\n");

    // Since we can't easily test output, just ensure no crash
    TEST_ASSERT_TRUE(t, 1);
}

void test_print_struct(test_context* t) {
    test_point_t point = {10, 20};
    print_generic(&test_point_meta, &point, file_stdout(), 0);
    print_string(file_stdout(), "\n");

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_array(test_context* t) {
    i32 arr[5] = {1, 2, 3, 4, 5};
    print_array_generic(&i32_meta, arr, arr + 5, file_stdout(), 0);
    print_string(file_stdout(), "\n");

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_to_file(test_context* t) {
    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

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
    mem_unmap(memory, stack_size);

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_nested_to_file(test_context* t) {
    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Define test data
    complex_t obj = {{10, 20}, 42};
    const char* expected = "complex_t {\n  pos: test_point_t {\n    x: 10\n    y: 20\n  }\n  id: 42\n}";

    // Open file for writing
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);

    // Print to file using file_t version
    print_generic(&complex_meta, &obj, file, 0);
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
    mem_unmap(memory, stack_size);

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_format_function(test_context* t) {
    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Open file for writing
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);

    // Test print_format function
    print_format(file, "Hello, World! Value: %d", 42);
    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_NULL(t, read_file);

    // Read content
    void* buffer;
    uptr size = file_read_all(read_file, &buffer, &alloc);
    TEST_ASSERT_TRUE(t, size > 0);

    // Check content
    const char* expected = "Hello, World! Value: 42";
    TEST_ASSERT_TRUE(t, strncmp((char*)buffer, expected, strlen(expected)) == 0);

    sa_free(&alloc, buffer);
    file_close(read_file);

    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    TEST_ASSERT_TRUE(t, 1);
}

void test_print_module(test_context* t) {
    setup_test_temp_dir();
    test_print_primitives(t);
    test_print_struct(t);
    test_print_array(t);
    test_print_to_file(t);
    test_print_nested_to_file(t);
    test_print_format_function(t);
    cleanup_test_temp_dir();
}
