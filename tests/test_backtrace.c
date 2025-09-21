#include "test_backtrace.h"

#include "test_temp_dir.h"
#include "stack_alloc.h"
#include "mem.h"
#include "print.h"
#include "backtrace.h"
#include "litteral.h"

static void test_backtrace_content(test_context* t) {
    setup_test_temp_dir();
    
    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Open file for writing
    const string file_path = STR("test_temp/backtrace.txt");
    file_t file = file_open(&alloc, file_path.begin, file_path.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    print_backtrace(file);

    // Print to file using file_t version
    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, file_path.begin, file_path.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, read_file, file_invalid());

    // Read content
    void* buffer;
    uptr size = file_read_all(read_file, &buffer, &alloc);
    TEST_ASSERT_TRUE(t, size > 0);

    // Check content
    const string backtrace_c = STR("backtrace.c");
    const string test_backtrace_content_expected = STR("test_backtrace_content");
    TEST_ASSERT_TRUE(t, sa_contains(&alloc, buffer, byteoffset(buffer, size), backtrace_c.begin, backtrace_c.end));
    TEST_ASSERT_TRUE(t, sa_contains(&alloc, buffer, byteoffset(buffer, size), test_backtrace_content_expected.begin, test_backtrace_content_expected.end));

    sa_free(&alloc, buffer);
    file_close(read_file);

    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    cleanup_test_temp_dir();

    TEST_ASSERT_TRUE(t, 1);
}

void test_backtrace_module(test_context* t) {
    REGISTER_TEST(t, "backtrace_content", test_backtrace_content);
}
