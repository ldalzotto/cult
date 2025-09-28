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

    TEST_ASSERT_EQUAL(t, command_count, 3);
    TEST_ASSERT_EQUAL(t, cmds[0].type, DRAW_COMMAND_CLEAR_BACKGROUND);
    TEST_ASSERT_EQUAL(t, cmds[0].data.clear_bg.color, 0x00000000);
    TEST_ASSERT_EQUAL(t, cmds[1].type, DRAW_COMMAND_DRAW_RECTANGLE);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.x, 50);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.y, 50);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.w, 5);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.h, 5);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.color, 0x00FFFFFF);

    TEST_ASSERT_EQUAL(t, cmds[2].type, DRAW_COMMAND_DRAW_RECTANGLE);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.x, 30);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.y, 30);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.w, 5);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.h, 5);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.color, 0x0000FF00);

    snake_deinit(s, alloc);

    sa_deinit(alloc);
    mem_unmap(pointer, size);
}

// New test: move the snake towards the reward and verify the reward is rendered at an expected position.
// Initial conditions based on snake_init: head starts at (10,10) in grid, reward at (6,6) in grid.
// We perform 2 left moves and 2 up moves to end at (8,8). With screen 100x100 and grid 20x20,
// cell size is 5, so head rect at (40,40) and reward rect at (30,30) in pixels.
void test_snake_move_towards_reward(test_context* t) {
    uptr size = 1024 * 1024;
    void* pointer = mem_map(size);
    stack_alloc _alloc;
    stack_alloc* alloc = &_alloc;
    sa_init(alloc, pointer, byteoffset(pointer, size));
    snake* s = snake_init(alloc);

    // Move left twice
    for (int i = 0; i < 2; ++i) {
        snake_input input = {0};
        input.left = 1;
        snake_update(s, input, 16, alloc);
    }
    // Move up twice
    for (int i = 0; i < 2; ++i) {
        snake_input input = {0};
        input.up = 1;
        snake_update(s, input, 16, alloc);
    }

    u32 command_count;
    draw_command* cmds = snake_render(s, 100, 100, &command_count, alloc);

    // Expected positions
    // Head: (10 - 2, 10 - 2) = (8, 8) -> 8 * 5 = 40
    // Reward: initial (6,6) -> 6 * 5 = 30
    TEST_ASSERT_EQUAL(t, command_count, 3);

    TEST_ASSERT_EQUAL(t, cmds[0].type, DRAW_COMMAND_CLEAR_BACKGROUND);

    TEST_ASSERT_EQUAL(t, cmds[1].type, DRAW_COMMAND_DRAW_RECTANGLE);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.x, 40);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.y, 40);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.w, 5);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.h, 5);
    TEST_ASSERT_EQUAL(t, cmds[1].data.rect.color, 0x00FFFFFF);

    TEST_ASSERT_EQUAL(t, cmds[2].type, DRAW_COMMAND_DRAW_RECTANGLE);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.x, 30);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.y, 30);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.w, 5);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.h, 5);
    TEST_ASSERT_EQUAL(t, cmds[2].data.rect.color, 0x0000FF00);

    snake_deinit(s, alloc);

    sa_deinit(alloc);
    mem_unmap(pointer, size);
}

void test_snake_module(test_context* t) {
    REGISTER_TEST(t, "snake_render", test_render);
    REGISTER_TEST(t, "snake_move_towards_reward", test_snake_move_towards_reward);
}
