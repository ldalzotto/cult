#include "target_build.h"

#include "target_execution_list.h"
#include "print.h"
#include "exec_command.h"
#include "target_timestamp.h"

// TODO: we probably want to pass the command template as input?
u8 target_build_object(target* t, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    
    const string template = {t->template, t->deps};
    string c_file = {alloc->cursor, alloc->cursor};
    for (string* d = t->deps; (void*)d < t->end;) {
        const string c_extension = STR(".c");
        if (sa_contains(alloc, d->begin, d->end, c_extension.begin, c_extension.end)) {
            c_file = *d;
            break;
        }

        d = (void*)d->end;
    }

    directory_create_for_file(alloc, t->name.begin, t->name.end, DIR_MODE_PUBLIC);

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

u8 target_build_executable(target* t, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    
    const string template = {t->template, t->deps};
    
    string deps_as_command;
    deps_as_command.begin = alloc->cursor;
    for (string* d = t->deps; (void*)d < t->end;) {
        const string extension = STRING(".o");
        if (!sa_contains(alloc, d->begin, d->end, extension.begin, extension.end)) {
            goto next;
        }
        void* cursor = sa_alloc(alloc, bytesize(d->begin, d->end));
        sa_copy(alloc, d->begin, cursor, bytesize(d->begin, d->end));
        cursor = sa_alloc(alloc, 1);
        *(u8*)cursor = ' ';
        next:
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

    const string s = STR("x11_lib");
    if (sa_equals(alloc, t->name.begin, t->name.end, s.begin, s.end)) {
        int debug = 0;
        unused(debug);
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


u8 target_build(target* targets_begin, target* targets_end, target* target_to_build, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    u8 success = 1;
    // Compute the target execution list
    struct { target** begin; target** end; } execution_list;
    execution_list.begin = target_execution_list(targets_begin, targets_end, target_to_build, alloc);
    execution_list.end = alloc->cursor;

    for (target** target_pp = execution_list.end - 1; target_pp >= execution_list.begin; --target_pp) {
        target* t = *target_pp;
        print_format(file_stdout(), STRING("%s\n"), t->name);

        if (target_should_build(t, cache_dir, alloc)) {
            if (!t->build(t, cache_dir, alloc)) {
                success = 0;
                break;
            }
        }
    }

    sa_free(alloc, begin);
    return success;
}
