#include "test_snake.h"
#include "print.h"
#include "mem.h"
#include "../src/apps/snake/snake.h"

// Some test helper functions
static void check_draw_rect(test_context* t, draw_command cmd,
                                i32 gx, i32 gy, i32 w, i32 h, u32 color) {
    const i32 TEST_CELL_SIZE = 5;
    TEST_ASSERT_EQUAL(t, cmd.type, DRAW_COMMAND_DRAW_RECTANGLE);
    TEST_ASSERT_EQUAL(t, cmd.data.rect.x, gx * TEST_CELL_SIZE);
    TEST_ASSERT_EQUAL(t, cmd.data.rect.y, gy * TEST_CELL_SIZE);
    TEST_ASSERT_EQUAL(t, cmd.data.rect.w, w);
    TEST_ASSERT_EQUAL(t, cmd.data.rect.h, h);
    TEST_ASSERT_EQUAL(t, cmd.data.rect.color, color);
}

static void check_cell_is_player(test_context* t, draw_command cmd, i32 gx, i32 gy) {
    check_draw_rect(t, cmd, gx, gy, 5, 5, 0x00FFFFFF);
}

static void check_cell_is_reward(test_context* t, draw_command cmd, i32 gx, i32 gy) {
    check_draw_rect(t, cmd, gx, gy, 5, 5, 0x0000FF00);
}

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

    check_cell_is_player(t, cmds[1], 10, 10);
    check_cell_is_reward(t, cmds[2], 6, 6);

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

    // Use shared helper for positions
    check_cell_is_player(t, cmds[1], 8, 8);
    check_cell_is_reward(t, cmds[2], 6, 6);

    snake_deinit(s, alloc);

    sa_deinit(alloc);
    mem_unmap(pointer, size);
}

// New test: boundary left/top
// Move the snake left and then up to reach the (0,0) boundary and verify rendering coordinates are clamped.
void test_snake_boundary_left_top(test_context* t) {
    uptr size = 1024 * 1024;
    void* pointer = mem_map(size);
    stack_alloc _alloc;
    stack_alloc* alloc = &_alloc;
    sa_init(alloc, pointer, byteoffset(pointer, size));
    snake* s = snake_init(alloc);

    // Move left towards the left boundary (0,0)
    for (int i = 0; i < 20; ++i) {
        snake_input input = {0};
        input.left = 1;
        snake_update(s, input, 16, alloc);
    }
    // Move up towards the top boundary
    for (int i = 0; i < 20; ++i) {
        snake_input input = {0};
        input.up = 1;
        snake_update(s, input, 16, alloc);
    }

    u32 command_count;
    draw_command* cmds = snake_render(s, 100, 100, &command_count, alloc);

    // We expect head clamped at (0,0) and reward at initial (6,6) -> (30,30) in pixels
    TEST_ASSERT_EQUAL(t, command_count, 3);

    TEST_ASSERT_EQUAL(t, cmds[0].type, DRAW_COMMAND_CLEAR_BACKGROUND);

    check_cell_is_reward(t, cmds[2], 6, 6);
    // Also verify head is at 0,0
    check_cell_is_player(t, cmds[1], 0, 0);

    snake_deinit(s, alloc);

    sa_deinit(alloc);
    mem_unmap(pointer, size);
}

// New test: ensure snake grows when a reward is eaten
void test_snake_increase_size_on_reward(test_context* t) {
    uptr size = 1024 * 1024;
    void* pointer = mem_map(size);
    stack_alloc _alloc;
    stack_alloc* alloc = &_alloc;
    sa_init(alloc, pointer, byteoffset(pointer, size));
    snake* s = snake_init(alloc);

    // Move left 4 times: (10,10) -> (6,10)
    for (int i = 0; i < 4; ++i) {
        snake_input input = {0};
        input.left = 1;
        snake_update(s, input, 16, alloc);
    }
    // Move up 4 times: (6,10) -> (6,6) -> eat reward
    for (int i = 0; i < 4; ++i) {
        snake_input input = {0};
        input.up = 1;
        snake_update(s, input, 16, alloc);
    }

    u32 command_count;
    draw_command* cmds = snake_render(s, 100, 100, &command_count, alloc);

    // After consuming the reward, the snake length should be 2
    // Command layout: Clear, 2 snake rects, 1 reward rect => 4 commands
    TEST_ASSERT_EQUAL(t, command_count, 4);

    // Clear background
    TEST_ASSERT_EQUAL(t, cmds[0].type, DRAW_COMMAND_CLEAR_BACKGROUND);

    // First snake segment (head) should be at (6,6) -> (30,30)
    check_cell_is_player(t, cmds[1], 6, 6);

    // Second snake segment (previous head) at (6,7) -> (30,35)
    check_draw_rect(t, cmds[2], 6, 7, 5, 5, 0x00FFFFFF);

    // Reward relocated to (9,10) -> (45,50)
    check_cell_is_reward(t, cmds[3], 9, 10);

    snake_deinit(s, alloc);
    sa_deinit(alloc);
    mem_unmap(pointer, size);
}

void test_snake_module(test_context* t) {
    REGISTER_TEST(t, "snake_render", test_render);
    REGISTER_TEST(t, "snake_move_towards_reward", test_snake_move_towards_reward);
    REGISTER_TEST(t, "snake_boundary_left_top", test_snake_boundary_left_top);
    REGISTER_TEST(t, "snake_increase_size_on_reward", test_snake_increase_size_on_reward);
}
