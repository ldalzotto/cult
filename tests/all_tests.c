#include "test_framework.h"
#include "test_mem.h"
#include "test_stack_alloc.h"
#include "test_win_x11.h"
#include "test_file.h"
#include "test_print.h"
#include "../src/print.h"
#include "../src/file.h"

int main() {
    test_context ctx;

    // Reset test counters before running tests
    test_reset_context(&ctx);

    print_string("Test Suite Starting...\n", file_get_stdout());

    // Run all module tests
    test_mem_module(&ctx);
    test_sa_module(&ctx);
    test_win_x11_module(&ctx);
    test_file_module(&ctx);
    test_print_module(&ctx);

    // Report results
    test_report_context(&ctx);

    return (ctx.failed > 0) ? 1 : 0;
}
