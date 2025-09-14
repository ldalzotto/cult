#include "test_file.h"
#include "../src/file.h"
#include "../src/primitive.h"
#include "../src/mem.h"
#include "../src/litteral.h"
#include "../src/stack_alloc.h"
#include <stdio.h>
#include <string.h>

// Path definitions using STATIC_STRING
static const string_span test_temp_dir      = STATIC_STRING("test_temp");
static const string_span path_non_existent  = STATIC_STRING("test_temp/non_existent_file.txt");
static const string_span path_test_output   = STATIC_STRING("test_temp/test_output.txt");
static const string_span path_test_read_all = STATIC_STRING("test_temp/test_read_all.txt");
static const string_span path_test_size     = STATIC_STRING("test_temp/test_size.txt");

static void cleanup_test_temp_dir(void) {
    const uptr size = 1024;
    void* pointer = mem_map(size);
    stack_alloc tmp;
    sa_init(&tmp, pointer, byteoffset(pointer, size));
    directory_remove(&tmp, test_temp_dir.begin, test_temp_dir.end);
    sa_deinit(&tmp);
    mem_unmap(pointer, size);
}

static void setup_test_temp_dir(void) {
    cleanup_test_temp_dir();

    const uptr size = 1024;
    void* pointer = mem_map(size);
    stack_alloc tmp;
    sa_init(&tmp, pointer, byteoffset(pointer, size));

    // Create test_temp directory if it doesn't exist
    directory_create(&tmp, test_temp_dir.begin, test_temp_dir.end, DIR_MODE_DEFAULT);

    sa_deinit(&tmp);
    mem_unmap(pointer, size);
}

static void test_file_open_close(test_context* t) {
    // Test opening a non-existent file for reading (should fail)
    file_t file = file_open(path_non_existent.begin, path_non_existent.end, FILE_MODE_READ);
    TEST_ASSERT_NULL(t, file);

    // Test opening a file for writing (should succeed)
    file = file_open(path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);

    // Test closing the file
    file_close(file);
}

static void test_file_write_read(test_context* t) {
    void* memory = mem_map(1024);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, 1024));

    const char* test_data = "Hello, World!";
    uptr data_size = sizeof("Hello, World!") - 1;  // -1 to exclude null terminator

    // Write to file
    file_t file = file_open(path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);

    uptr bytes_written = file_write(file, test_data, data_size);
    TEST_ASSERT_EQUAL(t, bytes_written, data_size);
    file_close(file);

    // Read from file
    file = file_open(path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_NULL(t, file);

    void* buffer;
    uptr bytes_read = file_read_all(file, &buffer, &alloc);
    TEST_ASSERT_EQUAL(t, bytes_read, data_size);
    TEST_ASSERT_EQUAL(t, strcmp(buffer, test_data), 0);

    sa_free(&alloc, buffer);
    file_close(file);
    sa_deinit(&alloc);
}

static void test_file_read_all(test_context* t) {
    const char* test_data = "This is a test file content.";
    uptr data_size = sizeof("This is a test file content.") - 1;  // -1 to exclude null terminator

    // Write test data to file
    file_t file = file_open(path_test_read_all.begin, path_test_read_all.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);
    file_write(file, test_data, data_size);
    file_close(file);

    // Read entire file using stack allocator
    void* memory = mem_map(1024);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, 1024));

    file = file_open(path_test_read_all.begin, path_test_read_all.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_NULL(t, file);

    void* buffer = NULL;
    uptr bytes_read = file_read_all(file, &buffer, &alloc);
    TEST_ASSERT_EQUAL(t, bytes_read, data_size);
    TEST_ASSERT_NOT_NULL(t, buffer);
    TEST_ASSERT_EQUAL(t, strcmp((char*)buffer, test_data), 0);

    file_close(file);
    sa_free(&alloc, buffer);
    sa_deinit(&alloc);
    mem_unmap(memory, 1024);
}

static void test_file_size(test_context* t) {
    const char* test_data = "Size test data";
    uptr data_size = sizeof("Size test data") - 1;  // -1 to exclude null terminator

    // Write test data
    file_t file = file_open(path_test_size.begin, path_test_size.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);
    file_write(file, test_data, data_size);
    file_close(file);

    // Check file size
    file = file_open(path_test_size.begin, path_test_size.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_NULL(t, file);

    uptr size = file_size(file);
    TEST_ASSERT_EQUAL(t, size, data_size);

    file_close(file);
}

void test_file_module(test_context* t) {
    printf("Running File Module Tests...\n");

    // Set up temporary directory for tests
    setup_test_temp_dir();

    test_file_open_close(t);
    test_file_write_read(t);
    test_file_read_all(t);
    test_file_size(t);

    // Clean up temporary directory
    cleanup_test_temp_dir();
}
