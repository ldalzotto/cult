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
static void target_build_object(target* t, string cache_dir, stack_alloc* alloc) {
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
    debug_assert(c_file.begin != c_file.end);
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
}

static void target_build_executable(target* t, string cache_dir, stack_alloc* alloc) {
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
}


static u8 target_should_build(target* t, string cache_dir, stack_alloc* alloc) {
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

    // Create a simple target representing "foo.o" depending on "foo.h".
    target* file_target = sa_alloc(alloc, sizeof(*file_target));

    {
        const string name = STR("foo.o");
        file_target->name.begin = sa_alloc(alloc, bytesize(name.begin, name.end));
        sa_copy(alloc, name.begin, (void*)file_target->name.begin, bytesize(name.begin, name.end));
        file_target->name.end = alloc->cursor;
    }
    {
        file_target->deps = alloc->cursor;
        {
            const string dep = STR("foo.c");
            string* dep_current  = sa_alloc(alloc, sizeof(*dep_current));
            dep_current->begin = sa_alloc(alloc, bytesize(dep.begin, dep.end));
            sa_copy(alloc, dep.begin, (void*)dep_current->begin, bytesize(dep.begin, dep.end));
            dep_current->end = alloc->cursor;
        }
    }

    file_target->end = alloc->cursor;

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
            const string dep = file_target->name;
            string* dep_current  = sa_alloc(alloc, sizeof(*dep_current));
            dep_current->begin = sa_alloc(alloc, bytesize(dep.begin, dep.end));
            sa_copy(alloc, dep.begin, (void*)dep_current->begin, bytesize(dep.begin, dep.end));
            dep_current->end = alloc->cursor;
        }
    }
    executable_target->end = alloc->cursor;


    // Build the target
    if (target_should_build(file_target, cache_dir, alloc)) {
        target_build_object(file_target, cache_dir, alloc);
    }

    if (target_should_build(executable_target, cache_dir, alloc)) {
        target_build_executable(executable_target, cache_dir, alloc);
    }
    
    sa_free(alloc, file_target);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return 0;
}
