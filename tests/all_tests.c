#include "test_framework.h"
#include "test_mem.h"
#include "test_stack_alloc.h"
#include "test_win_x11.h"
#include "test_file.h"
#include "test_print.h"
#include "test_lzss.h"
#include "test_backtrace.h"
#include "test_fps_ticker.h"
#include "test_snake.h"
#include "test_network.h"

#include "print.h"
#include "file.h"
#include "mem.h"

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
        const string arg = {argv[i], byteoffset(argv[i], mem_cstrlen(argv[i]))};
        const string filter = STRING("--filter");
        const string help = STRING("--help");
        const string h = STRING("-h");
        if (sa_equals(&alloc, arg.begin, arg.end, filter.begin, filter.end) && i + 1 < argc) {
            ctx->filter_pattern.begin = argv[i + 1];
            char* end = argv[i + 1];
            while (*end) {++end;}
            ctx->filter_pattern.end = end;
            i++; // Skip the next argument
        } else if (sa_equals(&alloc, arg.begin, arg.end, help.begin, help.end) || 
                    sa_equals(&alloc, arg.begin, arg.end, h.begin, h.end) == 0) {
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
    test_backtrace_module(ctx);
    test_lzss_module(ctx);
    test_fps_ticker_module(ctx);
    test_snake_module(ctx);
    test_network_module(ctx);

    // Run filtered tests
    test_run_filtered(ctx);

    // Report results
    test_report_context(ctx);

    u8 result = ctx->failed;
    sa_free(&alloc, ctx);
    mem_unmap(pointer, size);

    return (result > 0) ? 1 : 0;
}
