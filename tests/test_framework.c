#include "test_framework.h"
#include "../src/print.h"
#include "../src/file.h"
#include <string.h>

void test_reset_context(test_context* t) {
    t->passed = 0;
    t->failed = 0;
    t->filter_pattern[0] = '\0';
    t->test_count = 0;
}

void test_report_context(test_context* t) {
    print_string(file_stdout(), "Test Results:\n");
    if (t->filter_pattern[0] != '\0') {
        print_format(file_stdout(), "  Filter: %s\n", t->filter_pattern);
    }
    print_format(file_stdout(), "  Passed: %u\n", t->passed);
    print_format(file_stdout(), "  Failed: %u\n", t->failed);
    print_format(file_stdout(), "  Total: %u\n", t->passed + t->failed);
}

void test_register(test_context* t, const char* name, void (*func)(test_context* t)) {
    if (t->test_count < MAX_TESTS) {
        strncpy(t->tests[t->test_count].name, name, MAX_TEST_NAME_LEN - 1);
        t->tests[t->test_count].name[MAX_TEST_NAME_LEN - 1] = '\0';
        t->tests[t->test_count].func = func;
        t->test_count++;
    }
}

static int test_matches_filter(const char* test_name, const char* pattern) {
    const char* name_ptr = test_name;
    const char* pattern_ptr = pattern;

    while (*name_ptr && *pattern_ptr) {
        if (*name_ptr != *pattern_ptr) {
            return 0;
        }
        name_ptr++;
        pattern_ptr++;
    }

    return 1;
}

void test_run_filtered(test_context* t) {
    u32 run_count = 0;
    u32 skipped_count = 0;

    for (u32 i = 0; i < t->test_count; i++) {
        int matches = (t->filter_pattern[0] == '\0') ||
                      test_matches_filter(t->tests[i].name, t->filter_pattern);

        if (matches) {
            print_format(file_stdout(), "Running test: %s\n", t->tests[i].name);
            t->tests[i].func(t);
            run_count++;
        } else {
            print_format(file_stdout(), "Skipping test: %s\n", t->tests[i].name);
            skipped_count++;
        }
    }

    if (t->filter_pattern[0] != '\0') {
        print_format(file_stdout(), "Tests run: %u, skipped: %u\n", run_count, skipped_count);
    }
}
