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

// TODO: we probably want to pass the command template as input?
static void target_build_object(target* t, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    const string template = STR("gcc -c %s -o %s");

    // We may want to find the first .c file among all deps?
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
        file_t file = file_open(alloc, c_file.begin, c_file.end, FILE_MODE_READ);
        uptr ts = file_modification_time(file);
        file_close(file);
        timestamp_write(t->name, cache_dir, ts, alloc);
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
    target* t = sa_alloc(alloc, sizeof(*t));

    {
        const string name = STR("foo.o");
        t->name.begin = sa_alloc(alloc, bytesize(name.begin, name.end));
        sa_copy(alloc, name.begin, (void*)t->name.begin, bytesize(name.begin, name.end));
        t->name.end = alloc->cursor;
    }

    // Single dependency array
    {
        t->deps = alloc->cursor;

        {
            const string dep = STR("foo.c");
            string* dep_current  = sa_alloc(alloc, sizeof(*dep_current));
            dep_current->begin = sa_alloc(alloc, bytesize(dep.begin, dep.end));
            sa_copy(alloc, dep.begin, (void*)dep_current->begin, bytesize(dep.begin, dep.end));
            dep_current->end = alloc->cursor;
        }
        // TODO: this should be computed by the compiler
        {
            const string dep = STR("foo.h");
            string* dep_current  = sa_alloc(alloc, sizeof(*dep_current));
            dep_current->begin = sa_alloc(alloc, bytesize(dep.begin, dep.end));
            sa_copy(alloc, dep.begin, (void*)dep_current->begin, bytesize(dep.begin, dep.end));
            dep_current->end = alloc->cursor;
        }
    
    }

    t->end = alloc->cursor;

    // Build the target
    if (target_should_build(t, cache_dir, alloc)) {
        target_build_object(t, cache_dir, alloc);
    }
    
    sa_free(alloc, t);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return 0;
}
