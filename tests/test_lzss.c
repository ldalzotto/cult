#include "test_lzss.h"
#include "print.h"
#include "mem.h"
#include "coding/lzss.h"
#include <string.h>

static void test_lzss(test_context* t) {
    uptr size = 1024 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    string input = STR("hello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_worldhello_world");
    uptr input_size = bytesize(input.begin, input.end);
    TEST_ASSERT(t, input_size > 0, "Input size should be positive");

    lzss_config config;
    config.match_size_max = 255;
    config.match_size_min = 3;
    config.window_size_max = 1024;
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size > 0, "Compressed size should be positive");
    TEST_ASSERT(t, compressed_size < input_size, "Compressed size should be smaller than input for repetitive data");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "");

    sa_free(&alloc, out);

    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_window_size_boundary(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Create input exactly equal to window_size_max with repetitions
    char pattern[] = "abc";
    uptr pattern_len = sizeof(pattern) - 1;
    uptr input_size = 1024; // window_size_max
    char* input_buf = sa_alloc(&alloc, input_size + 1);
    for (uptr i = 0; i < input_size; i++) {
        input_buf[i] = pattern[i % pattern_len];
    }
    input_buf[input_size] = '\0';
    string input = {input_buf, input_buf + input_size};

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_free(&alloc, input_buf);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_large_input(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Create input larger than window_size_max
    char pattern[] = "def";
    uptr pattern_len = sizeof(pattern) - 1;
    uptr input_size = 1200; // > 1024
    char* input_buf = sa_alloc(&alloc, input_size + 1);
    for (uptr i = 0; i < input_size; i++) {
        input_buf[i] = pattern[i % pattern_len];
    }
    input_buf[input_size] = '\0';
    string input = {input_buf, input_buf + input_size};

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_free(&alloc, input_buf);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_all_identical(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    uptr input_size = 512;
    char* input_buf = sa_alloc(&alloc, input_size);
    memset(input_buf, 'A', input_size);
    string input = {input_buf, input_buf + input_size};

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size < input_size, "Should compress well with identical bytes");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_free(&alloc, input_buf);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_alternating_pattern(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    uptr input_size = 256;
    char* input_buf = sa_alloc(&alloc, input_size);
    for (uptr i = 0; i < input_size; i++) {
        input_buf[i] = (i % 2 == 0) ? 'A' : 'B';
    }
    string input = {input_buf, input_buf + input_size};

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_free(&alloc, input_buf);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_minimum_match(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Input with exact minimum match size repetitions
    string input = STR("abcabcabcxyz");
    uptr input_size = bytesize(input.begin, input.end);

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_with_nulls(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Binary data with null bytes and repetitions
    u8 input_data[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 3, 4, 5};
    uptr input_size = sizeof(input_data);
    string input = {(char*)input_data, (char*)input_data + input_size};

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_end_match(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Match at the very end
    string input = STR("xyzabcabc");
    uptr input_size = bytesize(input.begin, input.end);

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_small_repetitions(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Repetitions smaller than minimum match size
    string input = STR("abababxy");
    uptr input_size = bytesize(input.begin, input.end);

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_config_variations(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    string input = STR("abcabcabcabc");
    uptr input_size = bytesize(input.begin, input.end);

    // Test with smaller window and different match sizes
    lzss_config config = {2, 10, 256}; // Smaller values
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

void test_lzss_module(test_context* t) {
    print_string(file_stdout(), STRING("Registering LZSS Module Tests...\n"));
    REGISTER_TEST(t, "lzss", test_lzss);
    REGISTER_TEST(t, "lzss_window_size_boundary", test_lzss_window_size_boundary);
    REGISTER_TEST(t, "lzss_large_input", test_lzss_large_input);
    REGISTER_TEST(t, "lzss_all_identical", test_lzss_all_identical);
    REGISTER_TEST(t, "lzss_alternating_pattern", test_lzss_alternating_pattern);
    REGISTER_TEST(t, "lzss_minimum_match", test_lzss_minimum_match);
    REGISTER_TEST(t, "lzss_with_nulls", test_lzss_with_nulls);
    REGISTER_TEST(t, "lzss_end_match", test_lzss_end_match);
    REGISTER_TEST(t, "lzss_small_repetitions", test_lzss_small_repetitions);
    REGISTER_TEST(t, "lzss_config_variations", test_lzss_config_variations);
}
