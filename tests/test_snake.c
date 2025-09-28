#include "test_snake.h"
#include "print.h"
#include "mem.h"
#include "../src/apps/snake/snake.h"

// Test that checks the returned command of the renderer.
void test_render(test_context* t) {
    uptr size = 1024 * 1024;
    void* pointer = mem_map(size);
    stack_alloc _alloc;
    stack_alloc* alloc = &_alloc;
    sa_init(alloc, pointer, byteoffset(pointer, size));
    snake* s = snake_init(alloc);

    u32 command_count;
    draw_command* cmds = snake_render(s, 100, 100, &command_count, alloc);

    TEST_ASSERT_EQUAL(t, command_count, 2);
    TEST_ASSERT_EQUAL(t, cmds[0].type, DRAW_COMMAND_CLEAR_BACKGROUND);
    TEST_ASSERT_EQUAL(t, cmds[0].data.clear_bg.color, 0x00000000);
    TEST_ASSERT_EQUAL(t, cmds[1].type, DRAW_COMMAND_DRAW_RECTANGLE);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.x, 50);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.y, 50);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.w, 10);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.h, 10);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.color, 0x00FFFFFF);

    snake_deinit(s, alloc);

    sa_deinit(alloc);
    mem_unmap(pointer, size);
}

void test_snake_module(test_context* t) {
    REGISTER_TEST(t, "snake_render", test_render);
}
