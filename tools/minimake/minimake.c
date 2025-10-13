
#include "mem.h"

#include "file.h"
#include "target.h"
#include "target_build.h"
#include "target_c_dependencies.h"

void target_offset(target* t, uptr offset) {
    t->name.begin = byteoffset(t->name.begin, offset);
    t->name.end = byteoffset(t->name.end, offset);
    t->template = byteoffset(t->template, offset);
    t->deps = byteoffset(t->deps, offset);
    t->end = byteoffset(t->end, offset);
    for (string* d = t->deps; (void*)d < t->end;) {
        d->begin = byteoffset(d->begin, offset);
        d->end = byteoffset(d->end, offset);
        d = (void*)d->end;
    }
}

target* targets_offset(target* target, uptr offset, stack_alloc* alloc) {
    struct target* start = target;
    struct target* t = start;
    while (1) {
        target_offset(t, offset);

        if (t->end >= alloc->cursor) {
            break;
        }
        t = t->end;
    }

    return start;
}

/* Helper utilities to simplify target creation */
static target* create_target(stack_alloc* alloc, const string name, u8 (*build)(target*, string, stack_alloc*)) {
    target* t = sa_alloc(alloc, sizeof(*t));

    /* Copy name into the stack allocator */
    t->name.begin = sa_alloc(alloc, bytesize(name.begin, name.end));
    sa_copy(alloc, name.begin, (void*)t->name.begin, bytesize(name.begin, name.end));
    t->name.end = alloc->cursor;


    t->build = build;

    t->template = alloc->cursor;

    /* Initialize deps pointer to current cursor; caller will add deps and then finalize */
    t->deps = alloc->cursor;

    return t;
}

static void finish_target(target* t, stack_alloc* alloc) {
    t->end = alloc->cursor;
}

static void push_dep_string(stack_alloc* alloc, const string dep) {
    string* dep_current = sa_alloc(alloc, sizeof(*dep_current));
    dep_current->begin = sa_alloc(alloc, bytesize(dep.begin, dep.end));
    sa_copy(alloc, dep.begin, (void*)dep_current->begin, bytesize(dep.begin, dep.end));
    dep_current->end = alloc->cursor;
}

static target* create_c_object_targets(string build_object_template, string extract_dependency_template,
        string build_dir,
        string* sources_begin,
        string* sources_end,
        stack_alloc* alloc) 
{
    void* begin = alloc->cursor;
    for (string* source = sources_begin; source < sources_end; ++source) {
        void* var_begin = alloc->cursor;
        string object_path;
        object_path.begin = alloc->cursor;
        void* cursor = sa_alloc(alloc, bytesize(build_dir.begin, build_dir.end));
        sa_copy(alloc, build_dir.begin, cursor, bytesize(build_dir.begin, build_dir.end));
        cursor = sa_alloc(alloc, bytesize(source->begin, source->end));
        sa_copy(alloc, source->begin, cursor, bytesize(source->begin, source->end));
        cursor = sa_alloc(alloc, 2);
        *(u8*)cursor = '.';
        *((u8*)cursor + 1) = 'o';
        object_path.end = alloc->cursor;

        target* target = create_target(alloc, object_path, target_build_object);
        target->template = sa_alloc(alloc, bytesize(build_object_template.begin, build_object_template.end));
        sa_copy(alloc, build_object_template.begin, target->template, bytesize(build_object_template.begin, build_object_template.end));
        target->deps = extract_c_dependencies(*source, extract_dependency_template, alloc);
        finish_target(target, alloc);

        const uptr offset = bytesize(var_begin, target);
        sa_move_tail(alloc, target, var_begin);
        target = var_begin;
        target_offset(target, -offset);
    }
    return begin;
}

/*
    Minimake - minimal demonstration of a build target that can run a command
    and have dependencies. This file replaces the [TASK] comments by a tiny
    target abstraction and an example of invoking exec_command to build it.
*/
i32 main(void) {
    const uptr memory_size = 1024 * 1024;
    void* memory = mem_map(memory_size);

    stack_alloc _alloc;
    stack_alloc* alloc = &_alloc;
    sa_init(alloc, memory, (char*)memory + memory_size);

    const string build_dir = STR("build_minimake/");
    // directory_remove(alloc, build_dir.begin, build_dir.end);
    directory_create(alloc, build_dir.begin, build_dir.end, DIR_MODE_PUBLIC);
    const string cache_dir = STR("build_minimake/.minimake/");
    // directory_remove(alloc, cache_dir.begin, cache_dir.end);
    directory_create(alloc, cache_dir.begin, cache_dir.end, DIR_MODE_PUBLIC);

    const string build_object_template = STR("gcc -DDEBUG_ASSERTIONS_ENABLED=1 -c %s -o %s");
    const string extract_dependency_template = STR("gcc -DDEBUG_ASSERTIONS_ENABLED=1 -MM %s");
    const string link_object_template = STR("gcc %s -o %s");

    struct {target* begin; target* end;} targets;
    targets.begin = alloc->cursor;

    void* v = alloc->cursor;
    
    // Create c files
    struct {string* begin; string* end;} c_files;
    c_files.begin = alloc->cursor;

    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/assert.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/backtrace.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/convert.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/exec_command.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/file.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/format_iterator.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/mem.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/meta_iterator.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/print.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/stack_alloc.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/system_time.c");
    *(string*)sa_alloc(alloc, sizeof(string)) = STRING("src/libs/time.c");

    c_files.end = alloc->cursor;
    

    void* vend = alloc->cursor;

    /* Create targets using helper functions */
    target* c_object_targets_begin = 
    create_c_object_targets(build_object_template, extract_dependency_template, build_dir, c_files.begin, c_files.end, alloc);
    target* c_object_targets_end = alloc->cursor;

    /* executable "foo" depends on foo.o and bar.o */
    {
        target* executable_target = create_target(alloc, STRING("build_minimake/foo"), target_build_executable);

        executable_target->template = sa_alloc(alloc, bytesize(link_object_template.begin, link_object_template.end));
        sa_copy(alloc, link_object_template.begin, executable_target->template, bytesize(link_object_template.begin, link_object_template.end));

        executable_target->deps = alloc->cursor;
        for (target* c_object_target = c_object_targets_begin; c_object_target < c_object_targets_end;) {
            push_dep_string(alloc, c_object_target->name);
            c_object_target = c_object_target->end;
        }
        
        finish_target(executable_target, alloc);
    }

    sa_move_tail(alloc, vend, v);
    targets_offset(v, -bytesize(v, vend), alloc);

    targets.end = alloc->cursor;

    target* target_to_build = 0;
    for (target* t = targets.begin; t < targets.end;) {
        const string target_to_build_name = STR("build_minimake/foo");
        if (sa_equals(alloc, t->name.begin, t->name.end, target_to_build_name.begin, target_to_build_name.end)) {
            target_to_build = t;
            break;
        }
        t = t->end;
    }

    i32 return_code = 0;
    if (!target_build(targets.begin, targets.end, target_to_build, cache_dir, alloc)) {
        return_code = 1;
    }
    
    sa_free(alloc, memory);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return return_code;
}
