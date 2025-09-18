#include "test_file.h"
#include "test_temp_dir.h"
#include "../src/file.h"
#include "../src/primitive.h"
#include "../src/mem.h"
#include "../src/litteral.h"
#include "../src/stack_alloc.h"
#include "../src/print.h"
#include <string.h>

// Path definitions using STR
static const string_span path_non_existent  = STR("test_temp/non_existent_file.txt");
static const string_span path_test_output   = STR("test_temp/test_output.txt");
static const string_span path_test_read_all = STR("test_temp/test_read_all.txt");
static const string_span path_test_size     = STR("test_temp/test_size.txt");

static void test_file_open_close(test_context* t) {
    // Set up temporary directory for tests
    setup_test_temp_dir();

    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Test opening a non-existent file for reading (should fail)
    file_t file = file_open(&alloc, path_non_existent.begin, path_non_existent.end, FILE_MODE_READ);
    TEST_ASSERT_EQUAL(t, file, file_invalid());

    // Test opening a file for writing (should succeed)
    file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    // Test closing the file
    file_close(file);

    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    // Clean up temporary directory
    cleanup_test_temp_dir();
}

static void test_file_write_read(test_context* t) {
    // Set up temporary directory for tests
    setup_test_temp_dir();

    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    const string_span test_data = STR("Hello, World!");
    uptr data_size = bytesize(test_data.begin, test_data.end);

    // Write to file
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    uptr bytes_written = file_write(file, test_data.begin, test_data.end);
    TEST_ASSERT_EQUAL(t, bytes_written, data_size);
    file_close(file);

    // Read from file
    file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    void* buffer;
    uptr bytes_read = file_read_all(file, &buffer, &alloc);
    TEST_ASSERT_EQUAL(t, bytes_read, data_size);
    TEST_ASSERT_EQUAL(t, memcmp(buffer, test_data.begin, data_size), 0);

    sa_free(&alloc, buffer);
    file_close(file);
    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    // Clean up temporary directory
    cleanup_test_temp_dir();
}

static void test_file_read_all(test_context* t) {
    // Set up temporary directory for tests
    setup_test_temp_dir();

    const string_span test_data = STR("This is a test file content.");
    uptr data_size = bytesize(test_data.begin, test_data.end);

    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Write test data to file
    file_t file = file_open(&alloc, path_test_read_all.begin, path_test_read_all.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());
    file_write(file, test_data.begin, test_data.end);
    file_close(file);

    // Read entire file using stack allocator
    file = file_open(&alloc, path_test_read_all.begin, path_test_read_all.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    void* buffer = NULL;
    uptr bytes_read = file_read_all(file, &buffer, &alloc);
    TEST_ASSERT_EQUAL(t, bytes_read, data_size);
    TEST_ASSERT_NOT_NULL(t, buffer);
    TEST_ASSERT_EQUAL(t, memcmp(buffer, test_data.begin, data_size), 0);

    file_close(file);
    sa_free(&alloc, buffer);
    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    // Clean up temporary directory
    cleanup_test_temp_dir();
}

static void test_file_size(test_context* t) {
    // Set up temporary directory for tests
    setup_test_temp_dir();

    const string_span test_data = STR("Size test data");
    uptr data_size = bytesize(test_data.begin, test_data.end);

    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Write test data
    file_t file = file_open(&alloc, path_test_size.begin, path_test_size.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());
    file_write(file, test_data.begin, test_data.end);
    file_close(file);

    // Check file size
    file = file_open(&alloc, path_test_size.begin, path_test_size.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    uptr size = file_size(file);
    TEST_ASSERT_EQUAL(t, size, data_size);

    file_close(file);
    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    // Clean up temporary directory
    cleanup_test_temp_dir();
}

void test_file_module(test_context* t) {
    print_string(file_stdout(), STR_SPAN("Registering File Module Tests...\n"));

    REGISTER_TEST(t, "file_open_close", test_file_open_close);
    REGISTER_TEST(t, "file_write_read", test_file_write_read);
    REGISTER_TEST(t, "file_read_all", test_file_read_all);
    REGISTER_TEST(t, "file_size", test_file_size);
}
