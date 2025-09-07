#include "test_framework.h"
#include <stdio.h>

void test_reset_context(test_context* t) {
    t->passed = 0;
    t->failed = 0;
}

void test_report_context(test_context* t) {
    printf("Test Results:\n");
    printf("  Passed: %u\n", t->passed);
    printf("  Failed: %u\n", t->failed);
    printf("  Total: %u\n", t->passed + t->failed);
}
