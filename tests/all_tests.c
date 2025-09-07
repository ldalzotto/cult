#include "../src/assert.h"
#include <stdio.h>

// Define test counters
int test_passed = 0;
int test_failed = 0;

// Forward declarations for test module functions
void test_mem_module();
void test_sa_module();

int main() {
    // Reset test counters before running tests
    test_reset();

    printf("Test Suite Starting...\n");

    // Run all module tests
    test_mem_module();
    test_sa_module();

    // Report results
    test_report();

    return (test_failed > 0) ? 1 : 0;
}
