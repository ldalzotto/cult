#include "test_print.h"
#include "test_temp_dir.h"
#include "../src/print.h"
#include "../src/litteral.h"
#include "../src/stack_alloc.h"
#include "../src/mem.h"
#include "../src/file.h"
#include "../src/meta_iterator.h"
#include <stddef.h>
#include <string.h>

// Path definitions using STR
static const string_span path_test_output = STR("test_temp/print_test_output.txt");

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
const field_descriptor test_point_fields[] = {
    {STR("x"), offsetof(test_point_t, x), &i32_meta},
    {STR("y"), offsetof(test_point_t, y), &i32_meta}
};

// Meta for test_point_t
const meta test_point_meta = {
    .type_name = STR("test_point_t"),
    .is_array = 0,
    .type_size = sizeof(test_point_t),
    .pt = PT_NONE,
    .fields = {
        ARRAY_RANGE(test_point_fields)
    },
};

// Fields for complex_t
const field_descriptor complex_fields[] = {
    {STR("pos"), offsetof(complex_t, pos), &test_point_meta},
    {STR("id"), offsetof(complex_t, id), &i32_meta}
};

// Meta for complex_t
const meta complex_meta = {
    .type_name = STR("complex_t"),
    .is_array = 0,
    .type_size = sizeof(complex_t),
    .pt = PT_NONE,
    .fields = {
        ARRAY_RANGE(complex_fields)
    }
};

static void test_print_primitives(test_context* t) {
    // Test printing primitives
    i32 val_i32 = 42;
    print_format(file_stdout(), "%m\n", &i32_meta, &val_i32);

    u64 val_u64 = 1234567890123456789ULL;
    print_format(file_stdout(), "%m\n", &u64_meta, &val_u64);

    // Since we can't easily test output, just ensure no crash
    TEST_ASSERT_TRUE(t, 1);
}

static void test_print_struct(test_context* t) {
    test_point_t point = {10, 20};
    print_format(file_stdout(), "%m\n", &test_point_meta, &point);

    TEST_ASSERT_TRUE(t, 1);
}

static void test_print_to_file(test_context* t) {
    setup_test_temp_dir();
    
    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Define test data
    test_point_t point = {10, 20};
    const char* expected = "test_point_t {x: 10, y: 20}";

    // Open file for writing
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    // Print to file using file_t version
    print_format(file, "%m", &test_point_meta, &point);
    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, read_file, file_invalid());

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

    cleanup_test_temp_dir();

    TEST_ASSERT_TRUE(t, 1);
}

static void test_print_nested_to_file(test_context* t) {
    setup_test_temp_dir();

    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Define test data
    complex_t obj = {{10, 20}, 42};
    const char* expected = "complex_t {pos: test_point_t {x: 10, y: 20}, id: 42}";

    print_format(file_stdout(), "%m\n", &complex_meta, &obj);
    
    // Open file for writing
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    // Print to file using file_t version
    print_format(file, "%m", &complex_meta, &obj);
    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

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

    cleanup_test_temp_dir();

    TEST_ASSERT_TRUE(t, 1);
}

static void test_print_format_function(test_context* t) {
    setup_test_temp_dir();

    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Open file for writing
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    // Test print_format function
    print_format(file, "Hello, World! Value: %d", 42);
    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

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

    cleanup_test_temp_dir();

    TEST_ASSERT_TRUE(t, 1);
}

static void test_print_format_meta_specifier(test_context* t) {
    setup_test_temp_dir();

    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Define test data
    test_point_t point = {10, 20};
    const char expected[] = "Point: test_point_t {x: 10, y: 20}";

    // Open file for writing
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    // Test print_format with %m specifier
    print_format(file, "Point: %m", &test_point_meta, &point);
    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    // Read content
    void* buffer;
    uptr size = file_read_all(read_file, &buffer, &alloc);
    TEST_ASSERT_TRUE(t, size > 0);

    // Check content
    TEST_ASSERT_TRUE(t, memcmp((char*)buffer, expected, sizeof(expected)) == 0);

    sa_free(&alloc, buffer);
    file_close(read_file);

    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    cleanup_test_temp_dir();

    TEST_ASSERT_TRUE(t, 1);
}

static void test_print_format_multiple_meta(test_context* t) {
    setup_test_temp_dir();
    
    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Define test data
    test_point_t point = {10, 20};
    complex_t obj = {{10, 20}, 42};
    const char expected[] = "Simple: test_point_t {x: 10, y: 20} Complex: complex_t {pos: test_point_t {x: 10, y: 20}, id: 42}";

    // Open file for writing
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    // Test print_format with multiple %m specifiers
    print_format(file, "Simple: %m Complex: %m", &test_point_meta, &point, &complex_meta, &obj);
    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    // Read content
    void* buffer;
    uptr size = file_read_all(read_file, &buffer, &alloc);
    TEST_ASSERT_TRUE(t, size > 0);

    // Check content
    TEST_ASSERT_TRUE(t, memcmp((char*)buffer, expected, sizeof(expected)) == 0);

    sa_free(&alloc, buffer);
    file_close(read_file);

    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    cleanup_test_temp_dir();

    TEST_ASSERT_TRUE(t, 1);
}

static void test_print_meta_iterator(test_context* t) {
    setup_test_temp_dir();

    // Allocate memory using mem_map
    const uptr stack_size = 1024;
    void* memory = mem_map(stack_size);
    stack_alloc alloc;
    sa_init(&alloc, memory, byteoffset(memory, stack_size));

    // Define expected output
    const char expected[] = "complex_t\nBegin\ntest_point_t\nBegin\ni32\nBegin\ntest_point_t\nMiddle\ni32\nBegin\ntest_point_t\nEnd\ncomplex_t\nMiddle\ni32\nBegin\ncomplex_t\nEnd\n";

    // Open file for writing
    file_t file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, file, file_invalid());

    print_meta_iterator* it = print_meta_iterator_init(&alloc, &complex_meta);

    while (1) {
        print_meta_iteration iteration = print_meta_iterator_next(it);
        if (!iteration.meta) {break;}

        print_format(file, "%s\n", iteration.meta->type_name);
        if (iteration.fields_current == iteration.meta->fields.begin) {
            print_format(file, "%s\n", "Begin");
        } else if (iteration.fields_current == iteration.meta->fields.end) {
            print_format(file, "%s\n", "End");
        } else {
            print_format(file, "%s\n", "Middle");
        }
    }
    print_meta_iterator_deinit(&alloc, it);

    file_close(file);

    // Open file for reading
    file_t read_file = file_open(&alloc, path_test_output.begin, path_test_output.end, FILE_MODE_READ);
    TEST_ASSERT_NOT_EQUAL(t, read_file, file_invalid());

    // Read content
    void* buffer;
    uptr size = file_read_all(read_file, &buffer, &alloc);
    TEST_ASSERT_TRUE(t, size > 0);

    // Check content
    TEST_ASSERT_TRUE(t, memcmp((char*)buffer, expected, sizeof(expected)) == 0);

    sa_free(&alloc, buffer);
    file_close(read_file);

    sa_deinit(&alloc);
    mem_unmap(memory, stack_size);

    cleanup_test_temp_dir();
}

void test_print_module(test_context* t) {
    REGISTER_TEST(t, "print_primitives", test_print_primitives);
    REGISTER_TEST(t, "print_struct", test_print_struct);
    REGISTER_TEST(t, "print_to_file", test_print_to_file);
    REGISTER_TEST(t, "print_nested_to_file", test_print_nested_to_file);
    REGISTER_TEST(t, "print_format_function", test_print_format_function);
    REGISTER_TEST(t, "print_format_meta_specifier", test_print_format_meta_specifier);
    REGISTER_TEST(t, "print_format_multiple_meta", test_print_format_multiple_meta);
    REGISTER_TEST(t, "print_meta_iterator", test_print_meta_iterator);
}
