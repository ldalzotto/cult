
#include "test_exec_command.h"
#include "test_temp_dir.h"
#include "exec_command.h"
#include "litteral.h"
#include "stack_alloc.h"
#include "mem.h"
#include "file.h"
#include "print.h"
#include <string.h>

// Paths for the files we'll create and then list
static const string path_exec_test_file1 = STR("test_temp/exec_test_file.txt");
static const string path_exec_test_file2 = STR("test_temp/exec_test_file2.txt");

static void test_exec_command_ls_lists_files_on_multiple_lines(test_context* t) {
    setup_test_temp_dir();

    // Create a small stack allocation for file operations
    const uptr stack_size_file = 1024;
    void* memory_file = mem_map(stack_size_file);
    stack_alloc alloc_file;
    sa_init(&alloc_file, memory_file, byteoffset(memory_file, stack_size_file));

    // Create two files inside test_temp so that `ls test_temp` will show multiple lines
    file_t f1 = file_open(&alloc_file, path_exec_test_file1.begin, path_exec_test_file1.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, f1, file_invalid());

    const char* content1 = "exec command test 1";
    file_write(f1, content1, byteoffset(content1, strlen(content1)));
    file_close(f1);

    file_t f2 = file_open(&alloc_file, path_exec_test_file2.begin, path_exec_test_file2.end, FILE_MODE_WRITE);
    TEST_ASSERT_NOT_EQUAL(t, f2, file_invalid());

    const char* content2 = "exec command test 2";
    file_write(f2, content2, byteoffset(content2, strlen(content2)));
    file_close(f2);

    // Allocate memory for exec_command result
    const uptr stack_size_exec = 4096;
    void* memory_exec = mem_map(stack_size_exec);
    stack_alloc alloc_exec;
    sa_init(&alloc_exec, memory_exec, byteoffset(memory_exec, stack_size_exec));

    // Run the command "ls test_temp"
    const string cmd = STR("ls test_temp");
    void* res = exec_command(cmd, &alloc_exec);

    print_string(file_stdout(), (string){res, alloc_exec.cursor});

    // Expect both filenames, each on its own line (ls outputs newline-separated names)
    const string compared = STR("exec_test_file2.txt\nexec_test_file.txt\n");
    TEST_ASSERT_TRUE(t, sa_equals(memory_exec, res, res, compared.begin, compared.end));

    sa_free(&alloc_exec, res);

    // Clean up
    sa_deinit(&alloc_exec);
    mem_unmap(memory_exec, stack_size_exec);

    sa_deinit(&alloc_file);
    mem_unmap(memory_file, stack_size_file);

    cleanup_test_temp_dir();

    TEST_ASSERT_TRUE(t, 1);
}

void test_exec_command_module(test_context* t) {
    REGISTER_TEST(t, "exec_command_ls_lists_files_on_multiple_lines", test_exec_command_ls_lists_files_on_multiple_lines);
}
