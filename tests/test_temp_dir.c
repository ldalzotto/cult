#include "test_temp_dir.h"
#include "../src/mem.h"
#include "../src/stack_alloc.h"
#include "../src/file.h"

const string_span test_temp_dir = STR("test_temp");

void cleanup_test_temp_dir(void) {
    const uptr stack_size = 1024;
    void* pointer = mem_map(stack_size);
    stack_alloc tmp;
    sa_init(&tmp, pointer, byteoffset(pointer, stack_size));
    directory_remove(&tmp, test_temp_dir.begin, test_temp_dir.end);
    sa_deinit(&tmp);
    mem_unmap(pointer, stack_size);
}

void setup_test_temp_dir(void) {
    cleanup_test_temp_dir();

    const uptr stack_size = 1024;
    void* pointer = mem_map(stack_size);
    stack_alloc tmp;
    sa_init(&tmp, pointer, byteoffset(pointer, stack_size));

    // Create test_temp directory if it doesn't exist
    directory_create(&tmp, test_temp_dir.begin, test_temp_dir.end, DIR_MODE_DEFAULT);

    sa_deinit(&tmp);
    mem_unmap(pointer, stack_size);
}
