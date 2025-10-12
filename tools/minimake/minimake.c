
#include "mem.h"

#include "file.h"
#include "target.h"
#include "target_build.h"
#include "target_c_dependencies.h"

static u8 target_build_dummy(target* t, string cache_dir, stack_alloc* alloc) {
    unused(t);
    unused(cache_dir);
    unused(alloc);
    return 1;
}

/* Helper utilities to simplify target creation */
static target* create_target(stack_alloc* alloc, const string name, u8 (*build)(target*, string, stack_alloc*)) {
    target* t = sa_alloc(alloc, sizeof(*t));

    /* Copy name into the stack allocator */
    t->name.begin = sa_alloc(alloc, bytesize(name.begin, name.end));
    sa_copy(alloc, name.begin, (void*)t->name.begin, bytesize(name.begin, name.end));
    t->name.end = alloc->cursor;

    t->build = build;

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
    directory_remove(alloc, build_dir.begin, build_dir.end);
    directory_create(alloc, build_dir.begin, build_dir.end, DIR_MODE_PUBLIC);
    const string cache_dir = STR(".minimake/");
    directory_remove(alloc, cache_dir.begin, cache_dir.end);
    directory_create(alloc, cache_dir.begin, cache_dir.end, DIR_MODE_PUBLIC);

    struct {target* begin; target* end;} targets;
    targets.begin = alloc->cursor;

    /* Create targets using helper functions */

    /* foo.o from foo.c (extract C deps) */
    {
        target* foo_o_target = create_target(alloc, STRING("build_minimake/foo.o"), target_build_object);
        extract_c_dependencies(STRING("foo.c"), alloc);
        finish_target(foo_o_target, alloc);
    }

    /* bar.o from bar.c (extract C deps) */
    {
        target* bar_o_target = create_target(alloc, STRING("build_minimake/bar.o"), target_build_object);
        extract_c_dependencies(STRING("bar.c"), alloc);
        finish_target(bar_o_target, alloc);
    }

    /* bar.h (dummy target, no deps) */
    {
        target* bar_h_target = create_target(alloc, STRING("bar.h"), target_build_dummy);
        finish_target(bar_h_target, alloc);
    }

    /* foo.h (dummy target, no deps) */
    {
        target* foo_h_target = create_target(alloc, STRING("foo.h"), target_build_dummy);
        finish_target(foo_h_target, alloc);
    }

    /* executable "foo" depends on foo.o and bar.o */
    {
        target* executable_target = create_target(alloc, STRING("build_minimake/foo"), target_build_executable);

        push_dep_string(alloc, STRING("build_minimake/foo.o"));
        push_dep_string(alloc, STRING("build_minimake/bar.o"));

        finish_target(executable_target, alloc);
    }

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

    target_build(targets.begin, targets.end, target_to_build, cache_dir, alloc);
    
    sa_free(alloc, memory);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return 0;
}
