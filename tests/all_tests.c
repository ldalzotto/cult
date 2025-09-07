#include "test_framework.h"
#include "test_mem.h"
#include "test_stack_alloc.h"
#include "test_win_x11.h"
#include "test_win_headless.h"
#include <stdio.h>

int main() {
    test_context ctx;

    // Reset test counters before running tests
    test_reset_context(&ctx);

    printf("Test Suite Starting...\n");

    // Run all module tests
    test_mem_module(&ctx);
    test_sa_module(&ctx);
    test_win_x11_module(&ctx);
    test_win_headless_module(&ctx);

    // Report results
    test_report_context(&ctx);

    return (ctx.failed > 0) ? 1 : 0;
}
