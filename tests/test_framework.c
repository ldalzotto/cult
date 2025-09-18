#include "test_framework.h"
#include "../src/print.h"
#include "../src/file.h"

test_context* test_context_init(stack_alloc* alloc) {
    test_context* t = sa_alloc(alloc, sizeof(*t));
    t->alloc = alloc;
    t->passed = 0;
    t->failed = 0;
    t->test_count = 0;
    t->filter_pattern = (string){0,0};
    t->entries = alloc->cursor;
    return t;
}

void test_report_context(test_context* t) {
    print_string(file_stdout(), STRING("Test Results:\n"));
    if (t->filter_pattern.begin) {
        print_format(file_stdout(), STRING("  Filter: %s\n"), t->filter_pattern);
    }
    print_format(file_stdout(), STRING("  Passed: %u\n"), t->passed);
    print_format(file_stdout(), STRING("  Failed: %u\n"), t->failed);
    print_format(file_stdout(), STRING("  Total: %u\n"), t->passed + t->failed);
}

void test_register(test_context* t, const string name, void (*func)(test_context* t)) {
    test_context_entry* test = sa_alloc(t->alloc, sizeof(*test));
    test->name = name;
    test->func = func;
    t->test_count++;
}

static int test_matches_filter(const string test_name, const string pattern) {
    const char* name_ptr = test_name.begin;
    const char* pattern_ptr = pattern.begin;

    while (name_ptr != test_name.end && pattern_ptr != pattern.end) {
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

    test_context_entry* test = t->entries;
    while ((void*)test < t->alloc->cursor) {
        int matches = (!t->filter_pattern.begin) ||
                      test_matches_filter(test->name, t->filter_pattern);

        if (matches) {
            print_format(file_stdout(), STRING("Running test: %s\n"), test->name);
            test->func(t);
            run_count++;
        } else {
            print_format(file_stdout(), STRING("Skipping test: %s\n"), test->name);
            skipped_count++;
        }

        ++test;
    }

    if (t->filter_pattern.begin) {
        print_format(file_stdout(), STRING("Tests run: %u, skipped: %u\n"), run_count, skipped_count);
    }
}
