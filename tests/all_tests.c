#include "test_framework.h"
#include "test_mem.h"
#include "test_stack_alloc.h"
#include "test_win_x11.h"
#include "test_file.h"
#include "test_print.h"
#include "../src/print.h"
#include "../src/file.h"
#include <string.h>

int main(int argc, char* argv[]) {
    test_context ctx;

    // Reset test counters before running tests
    test_reset_context(&ctx);

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--filter") == 0 && i + 1 < argc) {
            strncpy(ctx.filter_pattern, argv[i + 1], MAX_TEST_NAME_LEN - 1);
            ctx.filter_pattern[MAX_TEST_NAME_LEN - 1] = '\0';
            i++; // Skip the next argument
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_string(file_stdout(), "Usage: all_tests [--filter <pattern>]\n");
            print_string(file_stdout(), "  --filter <pattern>: Run only tests matching the pattern (supports * wildcards)\n");
            print_string(file_stdout(), "  --help: Show this help message\n");
            return 0;
        }
    }

    print_string(file_stdout(), "Test Suite Starting...\n");
    if (ctx.filter_pattern[0] != '\0') {
        print_format(file_stdout(), "Filter: %s\n", ctx.filter_pattern);
    }

    // Register all tests
    test_mem_module(&ctx);
    test_sa_module(&ctx);
    test_win_x11_module(&ctx);
    test_file_module(&ctx);
    test_print_module(&ctx);

    // Run filtered tests
    test_run_filtered(&ctx);

    // Report results
    test_report_context(&ctx);

    return (ctx.failed > 0) ? 1 : 0;
}
