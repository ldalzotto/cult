#include "test_lzss.h"
#include "print.h"
#include "mem.h"
#include "coding/lzss.h"

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

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 20, "Compression should work");
    TEST_ASSERT(t, compressed_size <= input_size, "Compressed size should not exceed input for repeating pattern");

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

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 20, "Compression should work");
    TEST_ASSERT(t, compressed_size <= input_size, "Compressed size should not exceed input for repeating pattern");

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
    sa_set(&alloc, input_buf, input_buf + input_size, 'A');
    string input = {input_buf, input_buf + input_size};

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 12, "Should compress well with identical bytes");

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

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 7, "Compression should work");
    TEST_ASSERT(t, compressed_size <= input_size, "Compressed size should not exceed input for alternating pattern");

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

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 4, "Compression should work");
    TEST_ASSERT(t, compressed_size <= input_size, "Compressed size should not exceed input for minimum match test");

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

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 4, "Compression should work");
    TEST_ASSERT(t, compressed_size > 0, "Compressed size should be > 0 for binary data with nulls");
    TEST_ASSERT(t, compressed_size <= input_size, "Compressed size should not exceed input for minimum match test");

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

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 4, "Compression should work");
    TEST_ASSERT(t, compressed_size <= input_size, "Compressed size should not exceed input for end-match test");

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

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 4, "Compression should work");
    TEST_ASSERT(t, compressed_size > 0, "Compressed size should be > 0 for small repetitions");
    /* For repetitions smaller than match_size_min, compression may not reduce size,
       so only check that we can compress/decompress correctly. */
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

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 8, "Compression should work");
    TEST_ASSERT(t, compressed_size <= input_size, "Compressed size should not exceed input for config variations");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_very_small_window(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    string input = STR("aaaabbbb");
    uptr input_size = bytesize(input.begin, input.end);

    // Test with very small window
    lzss_config config = {3, 255, 4}; // Very small window
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 4, "Compression should work");
    TEST_ASSERT(t, compressed_size > 0, "Compressed size should be > 0 for very small window");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_match_size_max_boundary(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    // Create input with a match exactly equal to match_size_max
    uptr match_len = 255; // match_size_max
    char* input_buf = sa_alloc(&alloc, match_len * 2 + 10);
    sa_set(&alloc, input_buf, input_buf + match_len, 'A');
    sa_set(&alloc, input_buf + match_len, input_buf + match_len + match_len, 'A');
    sa_set(&alloc, input_buf + match_len * 2, input_buf + match_len * 2 + 10, 'X');
    string input = {input_buf, input_buf + match_len * 2 + 10};
    uptr input_size = bytesize(input.begin, input.end);

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 12, "Compression should work");
    TEST_ASSERT(t, compressed_size <= input_size, "Compressed size should not exceed input for match boundary");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_free(&alloc, input_buf);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_long_identical_run(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    uptr input_size = 1000;
    char* input_buf = sa_alloc(&alloc, input_size);
    sa_set(&alloc, input_buf, input_buf + input_size, 'Z');
    string input = {input_buf, input_buf + input_size};

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 16, "Compression should work");
    TEST_ASSERT(t, compressed_size < input_size, "Should compress extremely well with long identical run");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_free(&alloc, input_buf);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_random_binary(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    uptr input_size = 512;
    u8* input_buf = sa_alloc(&alloc, input_size);
    // Fill with pseudo-random data
    for (uptr i = 0; i < input_size; i++) {
        input_buf[i] = (u8)(i * 7 + 13) % 256;
    }
    string input = {(char*)input_buf, (char*)input_buf + input_size};

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 12, "Compression should work");
    TEST_ASSERT(t, compressed_size > 0, "Compressed size should be > 0 for random binary");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_free(&alloc, input_buf);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_debug_output(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    string input = STR("abcabcabcxyz");
    uptr input_size = bytesize(input.begin, input.end);

    lzss_config config = {3, 255, 1024};
    // Test with debug output (using stdout for simplicity)
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, file_stdout());
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 4, "Compression should work");
    TEST_ASSERT(t, compressed_size > 0, "Compressed size should be > 0 when using debug output");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, file_stdout());
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_empty_input(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    string input = STR("");
    uptr input_size = bytesize(input.begin, input.end);

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size == 0, "");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match empty input (0)");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_lzss_single_byte(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    string input = STR("A");
    uptr input_size = bytesize(input.begin, input.end);

    lzss_config config = {3, 255, 1024};
    void* out = lzss_compress((u8*)input.begin, (u8*)input.end, config, &alloc, 0);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size > 0, "Compressed size for single byte should be > 0");

    void* decompressed = lzss_decompress(out, alloc.cursor, &alloc, 0);
    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompressed size should match single byte input");

    sa_free(&alloc, decompressed);
    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

void test_lzss_module(test_context* t) {
    print_string(file_stdout(), STRING("Registering LZSS Module Tests...\n"));
    REGISTER_TEST(t, "lzss_window_size_boundary", test_lzss_window_size_boundary);
    REGISTER_TEST(t, "lzss_large_input", test_lzss_large_input);
    REGISTER_TEST(t, "lzss_all_identical", test_lzss_all_identical);
    REGISTER_TEST(t, "lzss_alternating_pattern", test_lzss_alternating_pattern);
    REGISTER_TEST(t, "lzss_minimum_match", test_lzss_minimum_match);
    REGISTER_TEST(t, "lzss_with_nulls", test_lzss_with_nulls);
    REGISTER_TEST(t, "lzss_end_match", test_lzss_end_match);
    REGISTER_TEST(t, "lzss_small_repetitions", test_lzss_small_repetitions);
    REGISTER_TEST(t, "lzss_config_variations", test_lzss_config_variations);
    REGISTER_TEST(t, "lzss_very_small_window", test_lzss_very_small_window);
    REGISTER_TEST(t, "lzss_match_size_max_boundary", test_lzss_match_size_max_boundary);
    REGISTER_TEST(t, "lzss_long_identical_run", test_lzss_long_identical_run);
    REGISTER_TEST(t, "lzss_random_binary", test_lzss_random_binary);
    REGISTER_TEST(t, "lzss_debug_output", test_lzss_debug_output);
    REGISTER_TEST(t, "lzss_empty_input", test_lzss_empty_input);
    REGISTER_TEST(t, "lzss_single_byte", test_lzss_single_byte);
}
