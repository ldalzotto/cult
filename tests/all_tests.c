#include "test_framework.h"
#include <stdio.h>

// Forward declarations for test module functions
void test_mem_module(test_context* t);
void test_sa_module(test_context* t);

int main() {
    test_context ctx;

    // Reset test counters before running tests
    test_reset_context(&ctx);

    printf("Test Suite Starting...\n");

    // Run all module tests
    test_mem_module(&ctx);
    test_sa_module(&ctx);

    // Report results
    test_report_context(&ctx);

    return (ctx.failed > 0) ? 1 : 0;
}
