
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

typedef struct {string* begin; void* end;} strings;

static void push_string(const string s, stack_alloc* alloc) {
    string* sp = sa_alloc(alloc, sizeof(string));
    sp->begin = sa_alloc(alloc, bytesize(s.begin, s.end));
    sa_copy(alloc, s.begin, (void*)sp->begin, bytesize(s.begin, s.end));
    sp->end = alloc->cursor;
}

static void push_strings(const strings s, stack_alloc* alloc) {
    for (string* str = s.begin; (void*)str < s.end;) {
        push_string(*str, alloc);
        str = (void*)str->end;
    }
}

static void push_string_data(const string s, stack_alloc* alloc) {
    void* cursor = sa_alloc(alloc, bytesize(s.begin, s.end));
    sa_copy(alloc, s.begin, cursor, bytesize(s.begin, s.end));
}

static void make_executable_link_template(stack_alloc* alloc, const strings link_flags) {
    push_string_data(STRING("gcc"), alloc);
    *(u8*)sa_alloc(alloc, 1) = ' ';
    push_string_data(STRING("%s"), alloc);
    *(u8*)sa_alloc(alloc, 1) = ' ';
    for (string* flag = link_flags.begin; (void*)flag < link_flags.end;) {
        push_string_data(*flag, alloc);
        *(u8*)sa_alloc(alloc, 1) = ' ';
        flag = link_flags.end;
    }
    push_string_data(STRING("-o %s"), alloc);
    *(u8*)sa_alloc(alloc, 1) = ' ';
}

static void make_build_object_template(stack_alloc* alloc, const string cc, const strings flags) {
    push_string_data(cc, alloc);
    *(u8*)sa_alloc(alloc, 1) = ' ';
    for (string* flag = flags.begin; (void*)flag < flags.end;) {
        push_string_data(*flag, alloc);
        *(u8*)sa_alloc(alloc, 1) = ' ';
        flag = (void*)flag->end;
    }
    push_string_data(STRING("-c %s -o %s"), alloc);
}

static void make_extract_dependency_template(stack_alloc* alloc, const string cc, const strings flags) {
    push_string_data(cc, alloc);
    *(u8*)sa_alloc(alloc, 1) = ' ';
    for (string* flag = flags.begin; (void*)flag < flags.end;) {
        push_string_data(*flag, alloc);
        *(u8*)sa_alloc(alloc, 1) = ' ';
        flag = (void*)flag->end;
    }
    push_string_data(STRING("-MM %s"), alloc);
}

static target* create_c_object_targets(const string cc, const strings flags, string build_dir,
        strings sources,
        stack_alloc* alloc) 
{
    void* begin = alloc->cursor;
    for (string* source = sources.begin; (void*)source < sources.end;) {
        void* var_begin = alloc->cursor;
        string object_path; object_path.begin = alloc->cursor;
        push_string_data(build_dir, alloc);
        push_string_data(*source, alloc);
        push_string_data(STRING(".o"), alloc);
        object_path.end = alloc->cursor;

        string build_object_template; build_object_template.begin = alloc->cursor;
        make_build_object_template(alloc, cc, flags);
        build_object_template.end = alloc->cursor;

        string extract_dependency_template; extract_dependency_template.begin = alloc->cursor;
        make_extract_dependency_template(alloc, cc, flags);
        extract_dependency_template.end = alloc->cursor;

        target* target = create_target(alloc, object_path, target_build_object);
        target->template = sa_alloc(alloc, bytesize(build_object_template.begin, build_object_template.end));
        sa_copy(alloc, build_object_template.begin, target->template, bytesize(build_object_template.begin, build_object_template.end));
        target->deps = extract_c_dependencies(*source, extract_dependency_template, alloc);
        finish_target(target, alloc);

        const uptr offset = bytesize(var_begin, target);
        sa_move_tail(alloc, target, var_begin);
        target = var_begin;
        target_offset(target, -offset);

        source = (void*)source->end;
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
    const string cache_dir = STR("build_minimake/.minimake/");

    directory_create(alloc, build_dir.begin, build_dir.end, DIR_MODE_PUBLIC);
    directory_create(alloc, cache_dir.begin, cache_dir.end, DIR_MODE_PUBLIC);

    const string cc = STR("gcc");

    strings common_c_flags;
    common_c_flags.begin = alloc->cursor;
    push_string(STRING("-DDEBUG_ASSERTIONS_ENABLED=1"), alloc);
    push_string(STRING("-Isrc/libs"), alloc);
    common_c_flags.end = alloc->cursor;

    struct {target* begin; target* end;} targets;
    targets.begin = alloc->cursor;

    void* v = alloc->cursor;
    
    // Create c files
    strings common_c_files;
    common_c_files.begin = alloc->cursor;

    push_string(STRING("src/libs/assert.c"), alloc);
    push_string(STRING("src/libs/backtrace.c"), alloc);
    push_string(STRING("src/libs/convert.c"), alloc);
    push_string(STRING("src/libs/exec_command.c"), alloc);
    push_string(STRING("src/libs/file.c"), alloc);
    push_string(STRING("src/libs/format_iterator.c"), alloc);
    push_string(STRING("src/libs/mem.c"), alloc);
    push_string(STRING("src/libs/meta_iterator.c"), alloc);
    push_string(STRING("src/libs/print.c"), alloc);
    push_string(STRING("src/libs/stack_alloc.c"), alloc);
    push_string(STRING("src/libs/system_time.c"), alloc);
    push_string(STRING("src/libs/time.c"), alloc);

    common_c_files.end = alloc->cursor;

    strings coding_c_flags; coding_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    coding_c_flags.end = alloc->cursor;
    
    strings coding_c_files;
    coding_c_files.begin = alloc->cursor;

    push_string(STRING("src/libs/coding/lz_deserialize.c"), alloc);
    push_string(STRING("src/libs/coding/lz_match_brute.c"), alloc);
    push_string(STRING("src/libs/coding/lz_serialize.c"), alloc);
    push_string(STRING("src/libs/coding/lz_window.c"), alloc);
    push_string(STRING("src/libs/coding/lzss.c"), alloc);

    coding_c_files.end = alloc->cursor;

    strings network_c_flags; network_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    network_c_flags.end = alloc->cursor;

    strings network_c_files;
    network_c_files.begin = alloc->cursor;

    push_string(STRING("src/libs/network/tcp/tcp_connection.c"), alloc);
    push_string(STRING("src/libs/network/tcp/tcp_read_write.c"), alloc);
    push_string(STRING("src/libs/network/https/https_request.c"), alloc);

    network_c_files.end = alloc->cursor;

    strings network_link_flags; network_link_flags.begin = alloc->cursor;
    push_string(STRING("-lssl"), alloc);
    network_link_flags.end = alloc->cursor;

    void* vend = alloc->cursor;

    /* Create targets using helper functions */
    target* common_targets_begin = 
    create_c_object_targets(cc, common_c_flags, build_dir, common_c_files, alloc);
    target* common_targets_end = alloc->cursor;

    target* coding_targets_begin =
    create_c_object_targets(cc, coding_c_flags, build_dir, coding_c_files, alloc);
    target* coding_targets_end = alloc->cursor;

    target* network_targets_begin =
    create_c_object_targets(cc, network_c_flags, build_dir, network_c_files, alloc);
    target* network_targets_end = alloc->cursor;

    /* executable "foo" depends on foo.o and bar.o */
    {
        void* var_begin = alloc->cursor;

        strings link_flags; link_flags.begin = alloc->cursor;
        push_strings(network_link_flags, alloc);
        link_flags.end = alloc->cursor;
        
        // TODO: don't do that explicitely, move it inside a create_executable_target
        string link_object_template;
        link_object_template.begin = alloc->cursor;
        make_executable_link_template(alloc, link_flags);
        link_object_template.end = alloc->cursor;

        struct {target** begin; target** end;} deps;
        deps.begin = alloc->cursor;
        for (target* c_object_target = common_targets_begin; c_object_target < common_targets_end;) {
            *(void**)sa_alloc(alloc, sizeof(void*)) = c_object_target;
            c_object_target = c_object_target->end;
        }
        for (target* c_object_target = coding_targets_begin; c_object_target < coding_targets_end;) {
            *(void**)sa_alloc(alloc, sizeof(void*)) = c_object_target;
            c_object_target = c_object_target->end;
        }
        for (target* c_object_target = network_targets_begin; c_object_target < network_targets_end;) {
            *(void**)sa_alloc(alloc, sizeof(void*)) = c_object_target;
            c_object_target = c_object_target->end;
        }
        deps.end = alloc->cursor;

        void* var_end = alloc->cursor;

        target* executable_target = create_target(alloc, STRING("build_minimake/foo"), target_build_executable);

        executable_target->template = sa_alloc(alloc, bytesize(link_object_template.begin, link_object_template.end));
        sa_copy(alloc, link_object_template.begin, executable_target->template, bytesize(link_object_template.begin, link_object_template.end));

        executable_target->deps = alloc->cursor;
        for (target** dep = deps.begin; dep < deps.end; ++dep) {
            push_string((*dep)->name, alloc);
        }
        
        finish_target(executable_target, alloc);
        
        sa_move_tail(alloc, var_end, var_begin);
        targets_offset(var_begin, -bytesize(var_begin, var_end), alloc);
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
