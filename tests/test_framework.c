#include "test_framework.h"
#include "../src/print.h"
#include "../src/file.h"
#include <string.h>

static test_context_entry* next_entry(test_context_entry* self) {
    return (test_context_entry*)self->name_end;
}

test_context* test_context_init(stack_alloc* alloc) {
    test_context* t = sa_alloc(alloc, sizeof(*t));
    t->alloc = alloc;
    t->passed = 0;
    t->failed = 0;
    t->test_count = 0;
    t->filter_pattern = alloc->cursor;
    t->entries = alloc->cursor;
    return t;
}

void test_report_context(test_context* t) {
    print_string(file_stdout(), "Test Results:\n");
    if (t->filter_pattern_enabled) {
        print_format(file_stdout(), "  Filter: %s\n", t->filter_pattern);
    }
    print_format(file_stdout(), "  Passed: %u\n", t->passed);
    print_format(file_stdout(), "  Failed: %u\n", t->failed);
    print_format(file_stdout(), "  Total: %u\n", t->passed + t->failed);
}

void test_register(test_context* t, const char* name, void (*func)(test_context* t)) {
    test_context_entry* test = sa_alloc(t->alloc, sizeof(*test));
    uptr name_size = strlen(name) + 1;
    test->name_begin = sa_alloc(t->alloc, name_size);
    test->name_end = t->alloc->cursor;
    memcpy(test->name_begin, name, name_size);
    test->func = func;
    t->test_count++;
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

    test_context_entry* test = t->entries;
    while ((void*)test < t->alloc->cursor) {
        int matches = (!t->filter_pattern_enabled) ||
                      test_matches_filter(test->name_begin, t->filter_pattern);

        if (matches) {
            print_format(file_stdout(), "Running test: %s\n", test->name_begin);
            test->func(t);
            run_count++;
        } else {
            print_format(file_stdout(), "Skipping test: %s\n", test->name_begin);
            skipped_count++;
        }

        test = next_entry(test);
    }

    if (t->filter_pattern_enabled) {
        print_format(file_stdout(), "Tests run: %u, skipped: %u\n", run_count, skipped_count);
    }
}
