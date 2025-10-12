
#include "primitive.h"
#include "mem.h"
#include "stack_alloc.h"
#include "print.h"
#include "file.h"
#include "exec_command.h"
#include "assert.h"

/*
    Minimake - minimal demonstration of a build target that can run a command
    and have dependencies. This file replaces the [TASK] comments by a tiny
    target abstraction and an example of invoking exec_command to build it.
*/

static void timestamp_write(const string name, const string cache_dir, uptr ts, stack_alloc* alloc) {
    string name_path;
    name_path.begin = sa_alloc(alloc, bytesize(cache_dir.begin, cache_dir.end));
    sa_copy(alloc, cache_dir.begin, (void*)name_path.begin, bytesize(cache_dir.begin, cache_dir.end));
    void* cursor = sa_alloc(alloc, bytesize(name.begin, name.end));
    sa_copy(alloc, name.begin, cursor, bytesize(name.begin, name.end));
    name_path.end = alloc->cursor;

    directory_create_for_file(alloc, name_path.begin, name_path.end, DIR_MODE_PUBLIC);
    file_t file = file_open(alloc, name_path.begin, name_path.end, FILE_MODE_WRITE);

    file_write(file, &ts, (&ts) + 1);
    file_close(file);

    sa_free(alloc, (void*)name_path.begin);
}

static uptr timestamp_read(const string name, const string cache_dir, stack_alloc* alloc) {
    string name_path;
    name_path.begin = sa_alloc(alloc, bytesize(cache_dir.begin, cache_dir.end));
    sa_copy(alloc, cache_dir.begin, (void*)name_path.begin, bytesize(cache_dir.begin, cache_dir.end));
    void* cursor = sa_alloc(alloc, bytesize(name.begin, name.end));
    sa_copy(alloc, name.begin, cursor, bytesize(name.begin, name.end));
    name_path.end = alloc->cursor;

    uptr ts = 0;
    file_t file = file_open(alloc, name_path.begin, name_path.end, FILE_MODE_READ);
    if (file != file_invalid()) {
        ts = file_modification_time(file);
        file_close(file);
    }

    sa_free(alloc, (void*)name_path.begin);

    return ts;
}

static string* extract_c_dependencies(string name, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    const string template = STR("gcc -MM %s");
    string command;
    command.begin = print_format_to_buffer(alloc, template, name);
    command.end = alloc->cursor;
    exec_command_result result = exec_command(command, alloc);

    string* deps_start = alloc->cursor;
    
    if (result.success) {
        string result_stdout;
        result_stdout.begin = result.output;
        result_stdout.end = alloc->cursor;

        /*
            Parse gcc -MM output and extract file paths (dependencies).
            The output looks like:
                build/tests/all_tests.d: tests/all_tests.c tests/test_framework.h \
                    src/libs/primitive.h ...
            We want to collect all path tokens except the first token that ends with ':'.
            Tokens are separated by whitespace; backslashes are used for line continuation
            and should be treated as whitespace.
        */

        // mark the start of the deps array in the allocator so callers can store it
        deps_start = alloc->cursor;
        // track whether we've skipped the "target:" token
        u8 skipped_target = 0;

        char* p = (char*)result_stdout.begin;
        char* e = (char*)result_stdout.end;
        while (p < e) {
            // skip whitespace and backslashes (line continuations)
            while (p < e && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\\')) { p++; }
            if (p >= e) break;

            char* tok_b = p;
            // token continues until whitespace or backslash
            while (p < e && !(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\\')) { p++; }
            char* tok_e = p;

            if (tok_e == tok_b) continue;

            // If this is the first token and it ends with ':', skip it (it's the target)
            if (!skipped_target && tok_e > tok_b && tok_e[-1] == ':') {
                skipped_target = 1;
                continue;
            }

            // If token ends with ':' (unexpected here), trim it
            if (tok_e > tok_b && tok_e[-1] == ':') {
                tok_e--;
                if (tok_e == tok_b) continue;
            }

            // Allocate a string entry for this dependency
            string* dep_entry = sa_alloc(alloc, sizeof(*dep_entry));
            dep_entry->begin = sa_alloc(alloc, bytesize(tok_b, tok_e));
            sa_copy(alloc, tok_b, (void*)dep_entry->begin, bytesize(tok_b, tok_e));
            dep_entry->end = alloc->cursor;
        }

    }

    string* deps_end = alloc->cursor;

    const uptr offset = bytesize(begin, deps_start);
    sa_move_tail(alloc, deps_start, begin);

    deps_start = byteoffset(deps_start, -offset);
    deps_end = byteoffset(deps_end, -offset);

    for (string* d = deps_start; d < deps_end;) {
        d->begin = byteoffset(d->begin, -offset);
        d->end = byteoffset(d->end, -offset);
        d = (void*)d->end;
    }

    return begin;
}

typedef struct {
    string name;
    string* deps;
    void* end;
} target;

static void target_update_timestamp(target* t, string cache_dir, stack_alloc* alloc) {
    uptr ts_to_write = 0;
    for (string* d = t->deps; (void*)d < t->end;) {
        uptr ts = timestamp_read(*d, cache_dir, alloc);
        if (ts > ts_to_write) {ts_to_write = ts;}
        d = (void*)d->end;
    }
    timestamp_write(t->name, cache_dir, ts_to_write, alloc);
}

// TODO: we probably want to pass the command template as input?
static u8 target_build_object(target* t, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    
    const string template = STR("gcc -c %s -o %s");
    string c_file = {alloc->cursor, alloc->cursor};
    for (string* d = t->deps; (void*)d < t->end;) {
        const string c_extension = STR(".c");
        if (sa_contains(alloc, d->begin, d->end, c_extension.begin, c_extension.end)) {
            c_file = *d;
            break;
        }

        d = (void*)d->end;
    }
    string command;
    command.begin = print_format_to_buffer(alloc, template, c_file, t->name);
    command.end = alloc->cursor;

    print_format(file_stdout(), STRING("%s\n"), command);

    exec_command_result exec = exec_command(command, alloc);
    string log;
    log.begin = exec.output;
    log.end = alloc->cursor;
    if (log.begin != log.end) {
        print_format(file_stdout(), STRING("%s\n"), log);
    }
    exec.output = 0;
    
    sa_free(alloc, begin);

    if (exec.success) {
        target_update_timestamp(t, cache_dir, alloc);
    }

    return exec.success;
}

static u8 target_build_executable(target* t, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    
    const string template = STR("gcc %s -o %s");
    
    string deps_as_command;
    deps_as_command.begin = alloc->cursor;
    for (string* d = t->deps; (void*)d < t->end;) {
        void* cursor = sa_alloc(alloc, bytesize(d->begin, d->end));
        sa_copy(alloc, d->begin, cursor, bytesize(d->begin, d->end));
        cursor = sa_alloc(alloc, 1);
        *(u8*)cursor = ' ';
        d = (void*)d->end;
    }
    deps_as_command.end = alloc->cursor;

    string command;
    command.begin = print_format_to_buffer(alloc, template, deps_as_command, t->name);
    command.end = alloc->cursor;

    print_format(file_stdout(), STRING("%s\n"), command);

    exec_command_result exec = exec_command(command, alloc);
    string log;
    log.begin = exec.output;
    log.end = alloc->cursor;
    if (log.begin != log.end) {
        print_format(file_stdout(), STRING("%s\n"), log);
    }
    exec.output = 0;
    
    sa_free(alloc, begin);

    if (exec.success) {
        target_update_timestamp(t, cache_dir, alloc);
    }
    return exec.success;
}


static u8 target_should_build(target* t, string cache_dir, stack_alloc* alloc) {
    if (t->deps == t->end) {
        return 1;
    }
    uptr input_ts = timestamp_read(t->name, cache_dir, alloc);
    for (const string* d = t->deps; (void*)d < t->end;) {
        uptr dep_ts = 0;
        file_t file = file_open(alloc, d->begin, d->end, FILE_MODE_READ);
        if (file != file_invalid()) {
            dep_ts = file_modification_time(file);
            file_close(file);
        } else {
            dep_ts = timestamp_read(*d, cache_dir, alloc);
        }
        
        if (dep_ts > input_ts) {
            return 1;
        }

        d = d->end;
    }
    return 0;
}

static target** target_execution_list(target* targets_begin, target* targets_end, target* main, stack_alloc* alloc) {

    /*
        Build a list of unique targets to execute, starting with 'main' as the
        first element. The list is stored in the allocator as an array of
        target* entries. Entries are appended as dependencies are discovered.
        The caller expects the list to be such that iterating from end-1 down
        to begin executes dependencies first and the main target last.
    */

    /* Reserve the start of the list in the allocator and push 'main' */
    target** list_begin = alloc->cursor;
    target** first = sa_alloc(alloc, sizeof(*first));
    *first = main;
    target** list_end = alloc->cursor;

    /* Iterate over the (growing) list and discover target dependencies */
    for (target** it = list_begin; it < list_end; ++it) {
        target* cur = *it;

        /* Iterate dependencies of cur */
        for (string* d = cur->deps; (void*)d < cur->end; ) {

            /* Try to find a target whose name matches this dependency */
            for (target* cand = targets_begin; cand < targets_end; cand = cand->end) {
                uptr cand_len = bytesize(cand->name.begin, cand->name.end);
                uptr dep_len = bytesize(d->begin, d->end);

                /* Quick length check */
                if (cand_len != dep_len) { /* not equal */ }
                else {
                    /* Compare bytes */
                    u8 equal = 1;
                    {
                        unsigned char* a = (unsigned char*)cand->name.begin;
                        unsigned char* b = (unsigned char*)d->begin;
                        uptr i;
                        for (i = 0; i < cand_len; ++i) {
                            if (a[i] != b[i]) { equal = 0; break; }
                        }
                    }

                    if (equal) {
                        /* Ensure uniqueness: check if cand is already in list by name */
                        u8 already = 0;
                        for (target** check = list_begin; check < list_end; ++check) {
                            target* existing = *check;
                            uptr exist_len = bytesize(existing->name.begin, existing->name.end);
                            if (exist_len != cand_len) { continue; }
                            u8 same = 1;
                            {
                                unsigned char* a = (unsigned char*)existing->name.begin;
                                unsigned char* b = (unsigned char*)cand->name.begin;
                                uptr i;
                                for (i = 0; i < cand_len; ++i) {
                                    if (a[i] != b[i]) { same = 0; break; }
                                }
                            }
                            if (same) { already = 1; break; }
                        }

                        if (!already) {
                            target** push = sa_alloc(alloc, sizeof(*push));
                            *push = cand;
                            list_end = alloc->cursor;
                        }

                        /* Found matching target, no need to check other candidates */
                        break;
                    }
                }
            }

            d = (void*)d->end;
        }
    }

    return list_begin;
}

i32 main(void) {
    const uptr memory_size = 1024 * 1024;
    void* memory = mem_map(memory_size);

    stack_alloc _alloc;
    stack_alloc* alloc = &_alloc;
    sa_init(alloc, memory, (char*)memory + memory_size);

    const string cache_dir = STR(".minimake/");
    directory_create(alloc, cache_dir.begin, cache_dir.end, DIR_MODE_PUBLIC);

    struct {target* begin; target* end;} targets;
    targets.begin = alloc->cursor;

    // Create a simple target representing "foo.o" depending on "foo.h".
    target* foo_o_target = sa_alloc(alloc, sizeof(*foo_o_target));

    {
        const string name = STR("foo.o");
        foo_o_target->name.begin = sa_alloc(alloc, bytesize(name.begin, name.end));
        sa_copy(alloc, name.begin, (void*)foo_o_target->name.begin, bytesize(name.begin, name.end));
        foo_o_target->name.end = alloc->cursor;
    }
    {
        foo_o_target->deps = alloc->cursor;
        {
            const string dep = STR("foo.c");
            extract_c_dependencies(dep, alloc);
        }
    }

    foo_o_target->end = alloc->cursor;


    target* bar_o_target = sa_alloc(alloc, sizeof(*bar_o_target));

    {
        const string name = STR("bar.o");
        bar_o_target->name.begin = sa_alloc(alloc, bytesize(name.begin, name.end));
        sa_copy(alloc, name.begin, (void*)bar_o_target->name.begin, bytesize(name.begin, name.end));
        bar_o_target->name.end = alloc->cursor;
    }
    {
        bar_o_target->deps = alloc->cursor;
        {
            const string dep = STR("bar.c");
            extract_c_dependencies(dep, alloc);
        }
    }

    bar_o_target->end = alloc->cursor;

    target* executable_target = sa_alloc(alloc, sizeof(*executable_target));
    {
        const string name = STR("foo");
        executable_target->name.begin = sa_alloc(alloc, bytesize(name.begin, name.end));
        sa_copy(alloc, name.begin, (void*)executable_target->name.begin, bytesize(name.begin, name.end));
        executable_target->name.end = alloc->cursor;
    }
    {
        executable_target->deps = alloc->cursor;
        {
            const string dep = foo_o_target->name;
            string* dep_current  = sa_alloc(alloc, sizeof(*dep_current));
            dep_current->begin = sa_alloc(alloc, bytesize(dep.begin, dep.end));
            sa_copy(alloc, dep.begin, (void*)dep_current->begin, bytesize(dep.begin, dep.end));
            dep_current->end = alloc->cursor;
        }
        {
            const string dep = bar_o_target->name;
            string* dep_current  = sa_alloc(alloc, sizeof(*dep_current));
            dep_current->begin = sa_alloc(alloc, bytesize(dep.begin, dep.end));
            sa_copy(alloc, dep.begin, (void*)dep_current->begin, bytesize(dep.begin, dep.end));
            dep_current->end = alloc->cursor;
        }
    }
    executable_target->end = alloc->cursor;

    targets.end = alloc->cursor;

    // Compute the target execution list
    struct { target** begin; target** end; } execution_list;
    execution_list.begin = target_execution_list(targets.begin, targets.end, executable_target, alloc);
    execution_list.end = alloc->cursor;

    for (target** target_pp = execution_list.end - 1; target_pp >= execution_list.begin; --target_pp) {
        target* t = *target_pp;
        print_format(file_stdout(), STRING("%s\n"), t->name);

        // TODO: Don't rely on that and have a function pointer
        const string o_extention = STR(".o");
        if (sa_contains(alloc, t->name.begin, t->name.end, o_extention.begin, o_extention.end)) {
            if (target_should_build(t, cache_dir, alloc)) {
                if (!target_build_object(t, cache_dir, alloc)) {
                    goto target_failed_to_build;
                }
            }
        } else {
            if (target_should_build(t, cache_dir, alloc)) {
                if (!target_build_executable(t, cache_dir, alloc)) {
                    goto target_failed_to_build;
                }
            }
        }
    }
    
target_failed_to_build:
    
    sa_free(alloc, foo_o_target);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return 0;
}
