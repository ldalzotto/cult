#include "./test_bmp.h"
#include "print.h"
#include "mem.h"
#include "image/bmp_read.h"
#include "image/bmp_write.h"

static void test_bmp_read_write(test_context* t) {
    // Create a small stack allocation for file operations
    const uptr stack_size_file = 1024 * 1024 * 1024;
    void* memory_file = mem_map(stack_size_file);
    stack_alloc _alloc; stack_alloc* alloc = &_alloc;
    sa_init(alloc, memory_file, byteoffset(memory_file, stack_size_file));

    bmp_file bmp_file_test;
    bmp_file_test.width = 24;
    bmp_file_test.height = 24 * 2;
    bmp_file_test.channels = 4;
    bmp_file_test.pixels = sa_alloc(alloc, bmp_file_test.width * bmp_file_test.height * bmp_file_test.channels);
    
    struct {void* begin; void* end;} result;
    result.begin = bmp_write(bmp_file_test, 0, alloc);
    result.end = alloc->cursor;

    bmp_file bmp = bmp_read(result.begin, result.end, alloc);

    TEST_ASSERT(t, bmp.width == bmp_file_test.width, "");
    TEST_ASSERT(t, bmp.height == bmp_file_test.height, "");
    TEST_ASSERT(t, bmp.channels == bmp_file_test.channels, "");

    sa_free(alloc, bmp.pixels);
    sa_free(alloc, result.begin);
    sa_free(alloc, bmp_file_test.pixels);
    
    sa_deinit(alloc);
    mem_unmap(memory_file, stack_size_file);
}

void test_bmp_module(test_context* t) {
    print_string(file_stdout(), STRING("Registering File Module Tests...\n"));

    REGISTER_TEST(t, "bmp_read_write", test_bmp_read_write);
}
