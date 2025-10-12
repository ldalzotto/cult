#include "mem.h"
#include "print.h"

#include "target.h"
#include "target_build.h"
#include "target_c_dependencies.h"
#include "target_execution_list.h"

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

    target_build(targets.begin, targets.end, executable_target, cache_dir, alloc);
    
    sa_free(alloc, foo_o_target);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return 0;
}
