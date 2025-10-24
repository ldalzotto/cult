
#include "mem.h"

#include "file.h"
#include "target.h"
#include "target_build.h"
#include "target_c_dependencies.h"
#include "exec_command.h"
#include "print.h"
#include "target_timestamp.h"

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

static void preprend_string_data(const string string_to_prepend, const string prepend_value, stack_alloc* alloc) {
    push_string_data(prepend_value, alloc);
    push_string_data(string_to_prepend, alloc);
}

static void make_executable_link_template(stack_alloc* alloc, const string cc, const strings link_flags) {
    push_string_data(cc, alloc);
    *(u8*)sa_alloc(alloc, 1) = ' ';
    push_string_data(STRING("%s"), alloc);
    *(u8*)sa_alloc(alloc, 1) = ' ';
    for (string* flag = link_flags.begin; (void*)flag < link_flags.end;) {
        push_string_data(*flag, alloc);
        *(u8*)sa_alloc(alloc, 1) = ' ';
        flag = (void*)flag->end;
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

static strings get_c_object_names(strings sources, string build_dir, stack_alloc* alloc) {
    strings names; names.begin = alloc->cursor;
    for (string* source = sources.begin; (void*)source<sources.end;) {
        string* s = sa_alloc(alloc, sizeof(*s));
        s->begin = alloc->cursor;
        preprend_string_data(*source, build_dir, alloc);
        s->end = alloc->cursor;
        *((u8*)s->end - 1) = 'o';
        source = (void*)source->end;
    }

    names.end = alloc->cursor;
    return names;
}

typedef struct {target* begin; void* end;} targets;

static targets create_c_object_targets(const string cc, const strings flags,
        strings sources,
        strings objects,
        strings additional_deps,
        stack_alloc* alloc) 
{
    void* begin = alloc->cursor;

    string* object = objects.begin;
    for (string* source = sources.begin; (void*)source < sources.end;) {
        void* var_begin = alloc->cursor;
        
        string build_object_template; build_object_template.begin = alloc->cursor;
        make_build_object_template(alloc, cc, flags);
        build_object_template.end = alloc->cursor;

        string extract_dependency_template; extract_dependency_template.begin = alloc->cursor;
        make_extract_dependency_template(alloc, cc, flags);
        extract_dependency_template.end = alloc->cursor;

        target* target = create_target(alloc, *object, target_build_object);
        target->template = sa_alloc(alloc, bytesize(build_object_template.begin, build_object_template.end));
        sa_copy(alloc, build_object_template.begin, target->template, bytesize(build_object_template.begin, build_object_template.end));
        target->deps = extract_c_dependencies(*source, extract_dependency_template, alloc);
        for (string* d = additional_deps.begin; (void*)d < additional_deps.end;) {
            push_string(*d, alloc);
            d = (void*)d->end;
        }
        finish_target(target, alloc);

        const uptr offset = bytesize(var_begin, target);
        sa_move_tail(alloc, target, var_begin);
        target = var_begin;
        target_offset(target, -offset);

        source = (void*)source->end;
        object = (void*)object->end;
    }
    return (targets){.begin = begin, alloc->cursor};
}

static target* create_executable_target(const string cc, const strings flags, string build_dir, string executable_file,
    strings deps,
     stack_alloc* alloc) {
    void* begin = alloc->cursor;
    void* var_begin = alloc->cursor;

    string name; name.begin = alloc->cursor;
    push_string_data(build_dir, alloc);
    push_string_data(executable_file, alloc);
    name.end = alloc->cursor;

    string executable_link_template; executable_link_template.begin = alloc->cursor;
    make_executable_link_template(alloc, cc, flags);
    executable_link_template.end = alloc->cursor;

    void* var_end = alloc->cursor;

    target* target = create_target(alloc, name, target_build_executable);
    target->template = sa_alloc(alloc, bytesize(executable_link_template.begin, executable_link_template.end));
    sa_copy(alloc, executable_link_template.begin, target->template, bytesize(executable_link_template.begin, executable_link_template.end));

    target->deps = alloc->cursor;
    for (string* dep = deps.begin; (void*)dep < deps.end;) {
        push_string(*dep, alloc);
        dep = (void*)dep->end;
    }

    finish_target(target, alloc);

    const uptr offset = bytesize(var_begin, var_end);
    sa_move_tail(alloc, var_end, var_begin);
    target_offset(begin, -offset);
    return begin;
}

static target* create_extract_target(const string targz_file, const string marker_file, const string timestamp_file, stack_alloc* alloc) {
    target* t = create_target(alloc, timestamp_file, target_build_extract);
    t->template = alloc->cursor;
    t->deps = alloc->cursor;
    
    push_string(targz_file, alloc);
    push_string(marker_file, alloc);
    finish_target(t, alloc);

    return t;
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

    targets targetss;
    targetss.begin = alloc->cursor;

    void* var_begin = alloc->cursor;

    const string cc = STR("gcc");

    strings common_c_flags;
    common_c_flags.begin = alloc->cursor;
    push_string(STRING("-DDEBUG_ASSERTIONS_ENABLED=1"), alloc);
    push_string(STRING("-Isrc/libs"), alloc);
    common_c_flags.end = alloc->cursor;
    
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

    strings common_o_files = get_c_object_names(common_c_files, build_dir, alloc);

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

    strings coding_o_files = get_c_object_names(common_c_files, build_dir, alloc);

    strings network_c_flags; network_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    network_c_flags.end = alloc->cursor;

    strings network_c_files;
    network_c_files.begin = alloc->cursor;

    push_string(STRING("src/libs/network/tcp/tcp_connection.c"), alloc);
    push_string(STRING("src/libs/network/tcp/tcp_read_write.c"), alloc);
    push_string(STRING("src/libs/network/https/https_request.c"), alloc);

    network_c_files.end = alloc->cursor;

    strings network_o_files = get_c_object_names(network_c_files, build_dir, alloc);

    u8 use_x11 = 0;
    {
        exec_command_result use_x11_result = exec_command(STRING("pkg-config --exists x11"), alloc);
        use_x11 = use_x11_result.success;
        sa_free(alloc, use_x11_result.output);
    }
    string x11_tgz; x11_tgz.begin = alloc->cursor;
    push_string_data(STRING("elibs/x11_headers.tar.gz"), alloc);
    x11_tgz.end = alloc->cursor;

    string x11_marker; x11_marker.begin = alloc->cursor;
    push_string_data(build_dir, alloc);
    push_string_data(STRING("elibs/X11/.marker"), alloc);
    x11_marker.end = alloc->cursor;

    string x11_timestamp; x11_timestamp.begin = alloc->cursor;
    push_string_data(build_dir, alloc);
    push_string_data(STRING("elibs/X11/.timestamp"), alloc);
    x11_timestamp.end = alloc->cursor;

    strings x11_link_flags; x11_link_flags.begin = alloc->cursor;
    if (use_x11) {
        push_string(STRING("-lX11"), alloc);
    }
    x11_link_flags.end = alloc->cursor;
    
    strings window_c_files; window_c_files.begin = alloc->cursor;
    push_string(STRING("src/libs/window/win_x11.c"), alloc);
    if (!use_x11) {
        push_string(STRING("src/libs/window/x11_stub.c"), alloc);
    }
    window_c_files.end = alloc->cursor;

    strings window_o_files = get_c_object_names(window_c_files, build_dir, alloc);

    strings window_c_flags; window_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    push_string(STRING("-I./src/elibs"), alloc);
    window_c_flags.end = alloc->cursor;

    strings window_deps; window_deps.begin = alloc->cursor;
    push_string(x11_timestamp, alloc);
    window_deps.end = alloc->cursor;

    strings dummy_c_files; dummy_c_files.begin = alloc->cursor;
    push_string(STRING("src/apps/dummy/dummy.c"), alloc);
    dummy_c_files.end = alloc->cursor;

    strings dummy_o_files = get_c_object_names(dummy_c_files, build_dir, alloc);

    strings dummy_c_flags; dummy_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    push_strings(window_c_flags, alloc);
    dummy_c_flags.end = alloc->cursor;

    strings dummy_link_flags; dummy_link_flags.begin = alloc->cursor;
    push_strings(x11_link_flags, alloc);
    dummy_link_flags.end = alloc->cursor;

    strings dummy_deps; dummy_deps.begin = alloc->cursor;
    push_strings(common_o_files, alloc);
    push_strings(window_o_files, alloc);
    push_strings(dummy_o_files, alloc);
    dummy_deps.end = alloc->cursor;

    strings snake_lib_c_files; snake_lib_c_files.begin = alloc->cursor;
    push_string(STRING("src/apps/snake/snake_grid.c"), alloc);
    push_string(STRING("src/apps/snake/snake_move.c"), alloc);
    push_string(STRING("src/apps/snake/snake_render.c"), alloc);
    push_string(STRING("src/apps/snake/snake_reward.c"), alloc);
    push_string(STRING("src/apps/snake/snake.c"), alloc);
    snake_lib_c_files.end = alloc->cursor;

    strings snake_lib_o_files = get_c_object_names(snake_lib_c_files, build_dir, alloc);

    strings snake_lib_c_flags;snake_lib_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    push_strings(window_c_flags, alloc);
    snake_lib_c_flags.end = alloc->cursor;

    strings snake_c_files;snake_c_files.begin = alloc->cursor;
    push_string(STRING("src/apps/snake/snake_loop.c"), alloc);
    snake_c_files.end = alloc->cursor;

    strings snake_o_files = get_c_object_names(snake_c_files, build_dir, alloc);

    strings snake_c_flags;snake_c_flags.begin = alloc->cursor;
    push_strings(snake_lib_c_flags, alloc);
    snake_c_flags.end = alloc->cursor;

    strings snake_link_flags; snake_link_flags.begin = alloc->cursor;
    push_strings(x11_link_flags, alloc);
    snake_link_flags.end = alloc->cursor;

    strings snake_deps; snake_deps.begin = alloc->cursor;
    push_strings(common_o_files, alloc);
    push_strings(window_o_files, alloc);
    push_strings(snake_lib_o_files, alloc);
    push_strings(snake_o_files, alloc);
    snake_deps.end = alloc->cursor;

    void* var_end = alloc->cursor;
    
    create_c_object_targets(cc, common_c_flags, common_c_files, common_o_files, (strings){0,0}, alloc);
    create_c_object_targets(cc, coding_c_flags, coding_c_files, coding_o_files, (strings){0,0}, alloc);
    create_c_object_targets(cc, network_c_flags, network_c_files, network_o_files, (strings){0,0}, alloc);

    // X11 lib target
    create_extract_target(x11_tgz, x11_marker, x11_timestamp, alloc);

    create_c_object_targets(cc, window_c_flags, window_c_files, window_o_files, window_deps, alloc);
    create_c_object_targets(cc, dummy_c_flags, dummy_c_files, dummy_o_files, (strings){0,0}, alloc);

    create_executable_target(cc, dummy_link_flags, build_dir, STRING("dummy"), dummy_deps, alloc);

    create_c_object_targets(cc, snake_lib_c_flags, snake_lib_c_files, snake_lib_o_files, (strings){0,0}, alloc);
    create_c_object_targets(cc, snake_c_flags, snake_c_files, snake_o_files, (strings){0,0}, alloc);

    create_executable_target(cc, snake_link_flags, build_dir, STRING("snake"), snake_deps, alloc);

    sa_move_tail(alloc, var_end, var_begin);
    targets_offset(var_begin, -bytesize(var_begin, var_end), alloc);

    targetss.end = alloc->cursor;

    target* target_dummy = 0;
    target* target_snake = 0;
    for (target* t = targetss.begin; (void*)t < targetss.end;) {
        const string target_dummy_name = STR("build_minimake/dummy");
        const string target_sname_name = STR("build_minimake/snake");
        if (sa_equals(alloc, t->name.begin, t->name.end, target_dummy_name.begin, target_dummy_name.end)) {
            target_dummy = t;
        }
        if (sa_equals(alloc, t->name.begin, t->name.end, target_sname_name.begin, target_sname_name.end)) {
            target_snake = t;
        }
        if (target_dummy && target_snake) {break;}
        t = t->end;
    }

    i32 return_code = 0;
    if (!target_build(targetss.begin, targetss.end, target_dummy, cache_dir, alloc)) {
        return_code = 1;
    }
    if (!target_build(targetss.begin, targetss.end, target_snake, cache_dir, alloc)) {
        return_code = 1;
    }
    
    sa_free(alloc, memory);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return return_code;
}
