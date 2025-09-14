#include "test_file.h"
#include "../src/file.h"
#include "../src/mem.h"
#include "../src/stack_alloc.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define TEST_TEMP_DIR "test_temp"

static void cleanup_test_temp_dir(void) {
    DIR* dir = opendir(TEST_TEMP_DIR);
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char filepath[256];
                snprintf(filepath, sizeof(filepath), "%s/%s", TEST_TEMP_DIR, entry->d_name);
                unlink(filepath);
            }
        }
        closedir(dir);
        rmdir(TEST_TEMP_DIR);
    }
}

static void setup_test_temp_dir(void) {
    cleanup_test_temp_dir();
    // Create test_temp directory if it doesn't exist
    mkdir(TEST_TEMP_DIR, 0755);
}

static void test_file_open_close(test_context* t) {
    // Test opening a non-existent file for reading (should fail)
    file_t file = file_open(TEST_TEMP_DIR "/non_existent_file.txt", FILE_MODE_READ);
    TEST_ASSERT_NULL(t, file);

    // Test opening a file for writing (should succeed)
    file = file_open(TEST_TEMP_DIR "/test_output.txt", FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);

    // Test closing the file
    file_close(file);
}

static void test_file_write_read(test_context* t) {
    void* memory = mem_map(1024);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, 1024));

    const char* test_data = "Hello, World!";
    uptr data_size = strlen(test_data);

    // Write to file
    file_t file = file_open(TEST_TEMP_DIR "/test_output.txt", FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);

    uptr bytes_written = file_write(file, test_data, data_size);
    TEST_ASSERT_EQUAL(t, bytes_written, data_size);

    file_close(file);

    // Read from file
    file = file_open(TEST_TEMP_DIR "/test_output.txt", FILE_MODE_READ);
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
    uptr data_size = strlen(test_data);

    // Write test data to file
    file_t file = file_open(TEST_TEMP_DIR "/test_read_all.txt", FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);
    file_write(file, test_data, data_size);
    file_close(file);

    // Read entire file using stack allocator
    void* memory = mem_map(1024);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, 1024));

    file = file_open(TEST_TEMP_DIR "/test_read_all.txt", FILE_MODE_READ);
    TEST_ASSERT_NOT_NULL(t, file);

    void* buffer = NULL;
    uptr bytes_read = file_read_all(file, &buffer, &alloc);
    TEST_ASSERT_EQUAL(t, bytes_read, data_size);
    TEST_ASSERT_NOT_NULL(t, buffer);
    TEST_ASSERT_EQUAL(t, strcmp((char*)buffer, test_data), 0);

    file_close(file);

    // Reset allocator cursor to beginning before deinit
    sa_free(&alloc, buffer);
    sa_deinit(&alloc);
    mem_unmap(memory, 1024);
}

static void test_file_size(test_context* t) {
    const char* test_data = "Size test data";
    uptr data_size = strlen(test_data);

    // Write test data
    file_t file = file_open(TEST_TEMP_DIR "/test_size.txt", FILE_MODE_WRITE);
    TEST_ASSERT_NOT_NULL(t, file);
    file_write(file, test_data, data_size);
    file_close(file);

    // Check file size
    file = file_open(TEST_TEMP_DIR "/test_size.txt", FILE_MODE_READ);
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
