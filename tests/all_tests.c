#include "test_framework.h"
#include "test_mem.h"
#include "test_stack_alloc.h"
#include "test_win_x11.h"
#include "test_file.h"
#include "test_print.h"
#include "print.h"
#include "file.h"
#include "mem.h"
#include <string.h>

int main(int argc, char* argv[]) {
    // Global memory pointer
    uptr size = 1024*2;
    void* pointer = mem_map(size);

    // Initialize the allocator
    stack_alloc alloc;
    sa_init(&alloc, pointer, byteoffset(pointer, size));

    // Reset test counters before running tests
    test_context* ctx = test_context_init(&alloc);

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--filter") == 0 && i + 1 < argc) {
            ctx->filter_pattern.begin = argv[i + 1];
            char* end = argv[i + 1];
            while (*end) {++end;}
            ctx->filter_pattern.end = end;
            i++; // Skip the next argument
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_string(file_stdout(), STRING("Usage: all_tests [--filter <pattern>]\n"));
            print_string(file_stdout(), STRING("  --filter <pattern>: Run only tests matching the pattern (supports * wildcards)\n"));
            print_string(file_stdout(), STRING("  --help: Show this help message\n"));
            return 0;
        }
    }

    print_string(file_stdout(), STRING("Test Suite Starting...\n"));
    if (ctx->filter_pattern.begin) {
        print_format(file_stdout(), STRING("Filter: %s\n"), ctx->filter_pattern);
    }

    // Register all tests
    test_mem_module(ctx);
    test_sa_module(ctx);
    test_win_x11_module(ctx);
    test_file_module(ctx);
    test_print_module(ctx);

    // Run filtered tests
    test_run_filtered(ctx);

    // Report results
    test_report_context(ctx);

    u8 result = ctx->failed;
    sa_free(&alloc, ctx);
    mem_unmap(pointer, size);

    return (result > 0) ? 1 : 0;
}
