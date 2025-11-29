#include "test_snake.h"
#include "print.h"
#include "mem.h"
#include "snake/snake.h"
#include "snake/snake_render.h"

/*
    The snake module performs a movement only when enough time us has been elapsed
    when calling snake_update(s, ..., delta_us, ...)
    This can be configured by using snake_set_config
*/

// A reusable snake test environment
typedef struct snake_test_env {
    void* pointer;
    uptr size;
    stack_alloc alloc;
    snake_asset* asset;
    snake_config config;
    i32 screen_w;
    i32 screen_h;
    i32 cell_size;
    snake* s;
} snake_test_env;

/* Create a reusable snake test environment (default 1MB pool) */
static snake_test_env env_init(void) {
    snake_test_env env;

    uptr size = 1024 * 1024;
    void* pointer = mem_map(size);
    stack_alloc alloc;
    sa_init(&alloc, pointer, byteoffset(pointer, size));

    env.s = snake_init(&alloc);

    env.pointer = pointer;
    env.size = size;
    env.alloc = alloc;

    env.asset = snake_asset_init(&alloc);

    env.config = (snake_config){ .delta_time_between_movement = 1 };
    snake_set_config(env.s, env.config);

    // Default screen dimensions
    env.screen_w = 100;
    env.screen_h = 100;

    // Cell size (pixels per grid cell)
    env.cell_size = 5;

    return env;
}

/* Destroy a previously created test environment */
static void env_deinit(snake_test_env* env) {
    snake_deinit(env->s, &env->alloc);
    sa_deinit(&env->alloc);
    mem_unmap(env->pointer, env->size);
}

static void env_update(snake_test_env* env, snake_input input) {
    snake_update(env->s, input, env->config.delta_time_between_movement, &env->alloc);
}

/* Render using the provided or internal allocator */
static draw_command* env_render(snake_test_env* env, u32* command_count) {
    return snake_render(env->s, env->asset, env->screen_w, env->screen_h, command_count, &env->alloc);
}

// Helper: draw a rectangle and verify its properties in the rendered command
static void check_draw_rect(test_context* t, snake_test_env* env, draw_command cmd, i32 gx, i32 gy, void* texture_data) {
    const i32 size = env->cell_size;
    TEST_ASSERT_EQUAL(t, cmd.type, DRAW_COMMAND_DRAW_RECTANGLE_TEXTURED);
    TEST_ASSERT_EQUAL(t, cmd.data.rect_textured.x, gx * size);
    TEST_ASSERT_EQUAL(t, cmd.data.rect_textured.y, gy * size);
    TEST_ASSERT_EQUAL(t, cmd.data.rect_textured.w, size);
    TEST_ASSERT_EQUAL(t, cmd.data.rect_textured.h, size);
    TEST_ASSERT_EQUAL(t, cmd.data.rect_textured.pixels, texture_data);
}

static void check_cell_is_player(test_context* t, snake_test_env* env, draw_command cmd, i32 gx, i32 gy) {
    check_draw_rect(t, env, cmd, gx, gy, snake_asset_body(env->asset).data);
}

static void check_cell_is_reward(test_context* t, snake_test_env* env, draw_command cmd, i32 gx, i32 gy) {
    check_draw_rect(t, env, cmd, gx, gy, snake_asset_apple(env->asset).data);
}

/*
    Note: The above utilities are intended to be shared across tests.
    The existing tests can use snake_test_env_create/destroy to establish
    a consistent environment and snake_test_env_render to obtain render commands.
*/

// Test that checks the returned command of the renderer.
static void test_render(test_context* t) {
    // Create a new test environment
    snake_test_env env = env_init();
    u32 command_count;
    draw_command* cmds = env_render(&env, &command_count);

    TEST_ASSERT_EQUAL(t, command_count, 3);
    TEST_ASSERT_EQUAL(t, cmds[0].type, DRAW_COMMAND_CLEAR_BACKGROUND);

    check_cell_is_player(t, &env, cmds[1], 10, 10);
    check_cell_is_reward(t, &env, cmds[2], 6, 6);

    // Cleanup
    env_deinit(&env);
}


// New test: move the snake towards the reward and verify the reward is rendered at an expected position.
// Initial conditions based on snake_init: head starts at (10,10) in grid, reward at (6,6) in grid.
// We perform 2 left moves and 2 up moves to end at (8,8). With screen 100x100 and grid 20x20,
// cell size is env.cell_size pixels, so head rect at (40,40) and reward rect at (30,30) in pixels.
static void test_snake_move_towards_reward(test_context* t) {
    snake_test_env env = env_init();

    // Move up twice
    for (int i = 0; i < 2; ++i) {
        snake_input input = {0};
        input.up = 1;
        env_update(&env, input);
    }
    // Move left twice
    for (int i = 0; i < 2; ++i) {
        snake_input input = {0};
        input.left = 1;
        env_update(&env, input);
    }

    u32 command_count;
    draw_command* cmds = env_render(&env, &command_count);

    // Expected positions
    // Head: (10 - 2, 10 - 2) = (8, 8) -> 8 * 5 = 40
    // Reward: initial (6,6) -> 6 * 5 = 30
    TEST_ASSERT_EQUAL(t, command_count, 3);

    TEST_ASSERT_EQUAL(t, cmds[0].type, DRAW_COMMAND_CLEAR_BACKGROUND);

    // Use shared helper for positions
    check_cell_is_player(t, &env, cmds[1], 8, 8);
    check_cell_is_reward(t, &env, cmds[2], 6, 6);

    env_deinit(&env);
}

// New test: boundary left/top
// Move the snake left and then up to reach the (0,0) boundary and verify rendering coordinates are clamped.
static void test_snake_boundary_left_top(test_context* t) {
    snake_test_env env = env_init();

    // Move up towards the top boundary
    for (int i = 0; i < 20; ++i) {
        snake_input input = {0};
        input.up = 1;
        env_update(&env, input);
    }
    // Move left towards the left boundary (0,0)
    for (int i = 0; i < 20; ++i) {
        snake_input input = {0};
        input.left = 1;
        env_update(&env, input);
    }

    u32 command_count;
    draw_command* cmds = env_render(&env, &command_count);

    // We expect head clamped at (0,0) and reward at initial (6,6) -> (30,30) in pixels
    TEST_ASSERT_EQUAL(t, command_count, 3);

    TEST_ASSERT_EQUAL(t, cmds[0].type, DRAW_COMMAND_CLEAR_BACKGROUND);

    check_cell_is_reward(t, &env, cmds[2], 6, 6);
    // Also verify head is at 0,0
    check_cell_is_player(t, &env, cmds[1], 0, 0);

    env_deinit(&env);
}

// New test: ensure snake grows when a reward is eaten
static void test_snake_increase_size_on_reward(test_context* t) {
    snake_test_env env = env_init();

    // Move up 4 times: (6,6) -> (6,10)
    for (int i = 0; i < 4; ++i) {
        snake_input input = {0};
        input.up = 1;
        env_update(&env, input);
    }
    // Move left 4 times: (6,10) -> (10,10) -> eat reward
    for (int i = 0; i < 4; ++i) {
        snake_input input = {0};
        input.left = 1;
        env_update(&env, input);
    }

    u32 command_count;
    draw_command* cmds = env_render(&env, &command_count);

    // After consuming the reward, the snake length should be 2
    // Command layout: Clear, 2 snake rects, 1 reward rect => 4 commands
    TEST_ASSERT_EQUAL(t, command_count, 4);

    // Clear background
    TEST_ASSERT_EQUAL(t, cmds[0].type, DRAW_COMMAND_CLEAR_BACKGROUND);

    // First snake segment (head) should be at (6,6) -> (30,30)
    check_cell_is_player(t, &env, cmds[1], 6, 6);

    // Second snake segment (previous head) at (6,7) -> (30,35)
    check_cell_is_player(t, &env, cmds[2], 7, 6);

    // Reward relocated
    check_cell_is_reward(t, &env, cmds[3], 5, 18);

    env_deinit(&env);
}

void test_snake_module(test_context* t) {
    REGISTER_TEST(t, "snake_render", test_render);
    REGISTER_TEST(t, "snake_move_towards_reward", test_snake_move_towards_reward);
    REGISTER_TEST(t, "snake_boundary_left_top", test_snake_boundary_left_top);
    REGISTER_TEST(t, "snake_increase_size_on_reward", test_snake_increase_size_on_reward);
}
