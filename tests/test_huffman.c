#include "test_huffman.h"
#include "print.h"
#include "mem.h"
#include "coding/huffman.h"

static void test_huffman_simple_repetitive(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    string input = STR("aaaabbbbccccddddeeeeaaaaabbbbbccccc");
    uptr input_size = bytesize(input.begin, input.end);

    void* out = huffman_compress((u8*)input.begin, (u8*)input.end, &alloc);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size > 0, "Compressed size should be > 0 for non-empty repetitive input");

    void* decompressed = huffman_decompress(out, alloc.cursor, &alloc);

    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompression should lead to same size.");
    TEST_ASSERT(t, sa_equals(&alloc, input.begin, input.end, decompressed, alloc.cursor), "Decompression should lead to same input.");

    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_huffman_random_binary(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    uptr input_size = 64;
    u8* input_buf = sa_alloc(&alloc, input_size);
    for (uptr i = 0; i < input_size; i++) {
        input_buf[i] = (u8)((i * 10 + 91) % 256);
    }
    string input = {(char*)input_buf, (char*)input_buf + input_size};

    void* out = huffman_compress((u8*)input.begin, (u8*)input.end, &alloc);
    TEST_ASSERT_NOT_NULL(t, out);

    uptr compressed_size = bytesize(out, alloc.cursor);
    TEST_ASSERT(t, compressed_size > 0, "Compressed size should be > 0 for random binary data");

    void* decompressed = huffman_decompress(out, alloc.cursor, &alloc);

    uptr decompressed_size = bytesize(decompressed, alloc.cursor);
    TEST_ASSERT(t, decompressed_size == input_size, "Decompression should lead to same size.");
    TEST_ASSERT(t, sa_equals(&alloc, input.begin, input.end, decompressed, alloc.cursor), "Decompression should lead to same input.");

    sa_free(&alloc, out);
    sa_free(&alloc, input_buf);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

static void test_huffman_empty_input(test_context* t) {
    uptr size = 64 * 1024;
    void* mem = mem_map(size);
    TEST_ASSERT_NOT_NULL(t, mem);

    stack_alloc alloc;
    sa_init(&alloc, mem, byteoffset(mem, size));

    string input = STR("");
    void* out = huffman_compress((u8*)input.begin, (u8*)input.end, &alloc);
    TEST_ASSERT_NOT_NULL(t, out);

    /* For empty input we only verify that the function handles it without crashing
       and returns a valid pointer. Specific compressed size/format may vary. */
    sa_free(&alloc, out);
    sa_deinit(&alloc);
    mem_unmap(mem, size);
}

void test_huffman_module(test_context* t) {
    print_string(file_stdout(), STRING("Registering Huffman Module Tests...\n"));
    REGISTER_TEST(t, "huffman_simple_repetitive", test_huffman_simple_repetitive);
    REGISTER_TEST(t, "huffman_random_binary", test_huffman_random_binary);
    REGISTER_TEST(t, "huffman_empty_input", test_huffman_empty_input);
}
