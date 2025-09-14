#include "test_framework.h"
#include "../src/print.h"
#include "../src/file.h"

void test_reset_context(test_context* t) {
    t->passed = 0;
    t->failed = 0;
}

void test_report_context(test_context* t) {
    print_string(file_get_stdout(), "Test Results:\n");
    print_format(file_get_stdout(), "  Passed: %u\n", t->passed);
    print_format(file_get_stdout(), "  Failed: %u\n", t->failed);
    print_format(file_get_stdout(), "  Total: %u\n", t->passed + t->failed);
}
