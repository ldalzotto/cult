#include "mem.h"
#include "print.h"
#include "exec_command.h"

#include "target.h"
#include "target_timestamp.h"
#include "target_c_dependencies.h"
#include "target_execution_list.h"

/*
    Minimake - minimal demonstration of a build target that can run a command
    and have dependencies. This file replaces the [TASK] comments by a tiny
    target abstraction and an example of invoking exec_command to build it.
*/

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
    foo_o_target->build = target_build_object;
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
    bar_o_target->build = target_build_object;
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
    executable_target->build = target_build_executable;
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

        if (target_should_build(t, cache_dir, alloc)) {
            if (!t->build(t, cache_dir, alloc)) {
                goto target_failed_to_build;
            }
        }
    }
    
target_failed_to_build:
    
    sa_free(alloc, foo_o_target);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return 0;
}
