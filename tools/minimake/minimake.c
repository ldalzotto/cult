
#include "mem.h"

#include "file.h"
#include "target.h"
#include "target_build.h"
#include "target_c_dependencies.h"
#include "exec_command.h"
#include "system_time.h"
#include "print.h"

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
static target* create_target(stack_alloc* alloc, const string name, u8 (*build)(target*, exec_command_session*, string, stack_alloc*)) {
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
        exec_command_session* session,
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
        target->deps = extract_c_dependencies(*source, extract_dependency_template, session, alloc);
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

    exec_command_session* session = open_persistent_shell(alloc);

    u64 setup_begin_ms = sys_time_ms();

    const string build_dir = STR("build_minimake/");
    const string cache_dir = STR("build_minimake/.minimake/");

    directory_create(alloc, build_dir.begin, build_dir.end, DIR_MODE_PUBLIC);
    directory_create(alloc, cache_dir.begin, cache_dir.end, DIR_MODE_PUBLIC);

    targets targetss;
    targetss.begin = alloc->cursor;

    void* var_begin = alloc->cursor;

    const string cc = STR("gcc");

    // BEGIN - common
    strings common_c_flags;
    common_c_flags.begin = alloc->cursor;
    push_string(STRING("-Wall"), alloc);
    push_string(STRING("-Wextra"), alloc);
    push_string(STRING("-Werror"), alloc);
    push_string(STRING("-pedantic"), alloc);
    push_string(STRING("-DDEBUG_ASSERTIONS_ENABLED=1"), alloc);
    push_string(STRING("-Isrc/libs"), alloc);
    common_c_flags.end = alloc->cursor;

    strings common_link_flags;
    common_link_flags.begin = alloc->cursor;
    push_string(STRING("-no-pie"), alloc);
    common_link_flags.end = alloc->cursor;
    
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
    // END - common

    // BEGIN - coding
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

    strings coding_o_files = get_c_object_names(coding_c_files, build_dir, alloc);
    // END - coding

    // BEGIN - network
    strings network_link_flags;network_link_flags.begin = alloc->cursor;
    push_string(STRING("-lssl"), alloc);
    network_link_flags.end = alloc->cursor;

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
    // END - network

    // BEGIN - x11
    u8 use_x11 = 0;
    {
        exec_command_result use_x11_result = command_session_exec_command(session, STRING("pkg-config --exists x11"), alloc);
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
    // END - x11

    // BEGIN - window
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
    // END - window

    // BEGIN - dummy
    strings dummy_c_files; dummy_c_files.begin = alloc->cursor;
    push_string(STRING("src/apps/dummy/dummy.c"), alloc);
    dummy_c_files.end = alloc->cursor;

    strings dummy_o_files = get_c_object_names(dummy_c_files, build_dir, alloc);

    strings dummy_c_flags; dummy_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    push_strings(window_c_flags, alloc);
    dummy_c_flags.end = alloc->cursor;

    strings dummy_link_flags; dummy_link_flags.begin = alloc->cursor;
    push_strings(common_link_flags, alloc);
    push_strings(x11_link_flags, alloc);
    dummy_link_flags.end = alloc->cursor;

    strings dummy_deps; dummy_deps.begin = alloc->cursor;
    push_strings(common_o_files, alloc);
    push_strings(window_o_files, alloc);
    push_strings(dummy_o_files, alloc);
    dummy_deps.end = alloc->cursor;
    // END - dummy

    // BEGIN - snake
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
    push_strings(common_link_flags, alloc);
    push_strings(x11_link_flags, alloc);
    snake_link_flags.end = alloc->cursor;

    strings snake_deps; snake_deps.begin = alloc->cursor;
    push_strings(common_o_files, alloc);
    push_strings(window_o_files, alloc);
    push_strings(snake_lib_o_files, alloc);
    push_strings(snake_o_files, alloc);
    snake_deps.end = alloc->cursor;
    // END - snake

    // BEGIN - agent
    strings agent_c_files;agent_c_files.begin = alloc->cursor;
    push_string(STRING("tools/agent/agent_args.c"), alloc);
    push_string(STRING("tools/agent/agent_request.c"), alloc);
    push_string(STRING("tools/agent/agent_result_write.c"), alloc);
    push_string(STRING("tools/agent/agent.c"), alloc);
    push_string(STRING("tools/agent/user_content_read.c"), alloc);
    agent_c_files.end = alloc->cursor;

    strings agent_o_files = get_c_object_names(agent_c_files, build_dir, alloc);

    strings agent_c_flags;agent_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    agent_c_flags.end = alloc->cursor;

    strings agent_link_flags;agent_link_flags.begin = alloc->cursor;
    push_strings(common_link_flags, alloc);
    push_strings(network_link_flags, alloc);
    agent_link_flags.end = alloc->cursor;

    strings agent_deps; agent_deps.begin =alloc->cursor;
    push_strings(common_o_files, alloc);
    push_strings(network_o_files, alloc);
    push_strings(agent_o_files, alloc);
    agent_deps.end = alloc->cursor;
    // END - agent

    // BEGIN - minimake
    strings minimake_c_files;minimake_c_files.begin = alloc->cursor;
    push_string(STRING("tools/minimake/minimake.c"), alloc);
    push_string(STRING("tools/minimake/target_build.c"), alloc);
    push_string(STRING("tools/minimake/target_c_dependencies.c"), alloc);
    push_string(STRING("tools/minimake/target_execution_list.c"), alloc);
    push_string(STRING("tools/minimake/target_timestamp.c"), alloc);
    minimake_c_files.end = alloc->cursor;

    strings minimake_o_files = get_c_object_names(minimake_c_files, build_dir, alloc);

    strings minimake_c_flags;minimake_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    minimake_c_flags.end = alloc->cursor;

    strings minimake_link_flags;minimake_link_flags.begin = alloc->cursor;
    push_strings(common_link_flags, alloc);
    minimake_link_flags.end = alloc->cursor;

    strings minimake_deps;minimake_deps.begin = alloc->cursor;
    push_strings(common_o_files, alloc);
    push_strings(minimake_o_files, alloc);
    minimake_deps.end = alloc->cursor;
    // END - minimake

    // BEGIN - tests
    strings tests_c_files; tests_c_files.begin = alloc->cursor;
    push_string(STRING("tests/all_tests.c"), alloc);
    push_string(STRING("tests/test_backtrace.c"), alloc);
    push_string(STRING("tests/test_exec_command.c"), alloc);
    push_string(STRING("tests/test_file.c"), alloc);
    push_string(STRING("tests/test_fps_ticker.c"), alloc);
    push_string(STRING("tests/test_framework.c"), alloc);
    push_string(STRING("tests/test_lzss.c"), alloc);
    push_string(STRING("tests/test_mem.c"), alloc);
    push_string(STRING("tests/test_network_https.c"), alloc);
    push_string(STRING("tests/test_network_tcp.c"), alloc);
    push_string(STRING("tests/test_print.c"), alloc);
    push_string(STRING("tests/test_snake.c"), alloc);
    push_string(STRING("tests/test_stack_alloc.c"), alloc);
    push_string(STRING("tests/test_temp_dir.c"), alloc);
    push_string(STRING("tests/test_win_x11.c"), alloc);
    tests_c_files.end = alloc->cursor;

    strings tests_o_files = get_c_object_names(tests_c_files, build_dir, alloc);

    strings tests_c_flags;tests_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    push_string(STRING("-Isrc/apps"), alloc);
    tests_c_flags.end = alloc->cursor;

    strings tests_link_flags;tests_link_flags.begin = alloc->cursor;
    push_strings(common_link_flags, alloc);
    push_strings(x11_link_flags, alloc);
    push_strings(network_link_flags, alloc);
    tests_link_flags.end = alloc->cursor;

    strings tests_deps;tests_deps.begin = alloc->cursor;
    push_strings(common_o_files, alloc);
    push_strings(window_o_files, alloc);
    push_strings(coding_o_files, alloc);
    push_strings(network_o_files, alloc);
    push_strings(snake_lib_o_files, alloc);
    push_strings(tests_o_files, alloc);
    tests_deps.end = alloc->cursor;
    // END - tests

    void* var_end = alloc->cursor;
    
    u64 setup_end_ms = sys_time_ms();

    u64 target_begin_ms = sys_time_ms();

    create_c_object_targets(cc, common_c_flags, common_c_files, common_o_files, (strings){0,0}, session, alloc);
    create_c_object_targets(cc, coding_c_flags, coding_c_files, coding_o_files, (strings){0,0}, session, alloc);
    create_c_object_targets(cc, network_c_flags, network_c_files, network_o_files, (strings){0,0}, session, alloc);

    // X11 lib target
    create_extract_target(x11_tgz, x11_marker, x11_timestamp, alloc);

    create_c_object_targets(cc, window_c_flags, window_c_files, window_o_files, window_deps, session, alloc);
    create_c_object_targets(cc, dummy_c_flags, dummy_c_files, dummy_o_files, (strings){0,0}, session, alloc);

    create_executable_target(cc, dummy_link_flags, build_dir, STRING("dummy"), dummy_deps, alloc);

    create_c_object_targets(cc, snake_lib_c_flags, snake_lib_c_files, snake_lib_o_files, (strings){0,0}, session, alloc);
    create_c_object_targets(cc, snake_c_flags, snake_c_files, snake_o_files, (strings){0,0}, session, alloc);
    create_executable_target(cc, snake_link_flags, build_dir, STRING("snake"), snake_deps, alloc);

    create_c_object_targets(cc, agent_c_flags, agent_c_files, agent_o_files, (strings){0,0}, session, alloc);
    create_executable_target(cc, agent_link_flags, build_dir, STRING("agent"), agent_deps, alloc);

    create_c_object_targets(cc, minimake_c_flags, minimake_c_files, minimake_o_files, (strings){0,0}, session, alloc);
    create_executable_target(cc, minimake_link_flags, build_dir, STRING("minimake"), minimake_deps, alloc);

    create_c_object_targets(cc, tests_c_flags, tests_c_files, tests_o_files, (strings){0,0}, session, alloc);
    create_executable_target(cc, tests_link_flags, build_dir, STRING("tests_run"), tests_deps, alloc);

    sa_move_tail(alloc, var_end, var_begin);
    targets_offset(var_begin, -bytesize(var_begin, var_end), alloc);

    targetss.end = alloc->cursor;

    u64 target_end_ms = sys_time_ms();

    u64 build_begin_ms = sys_time_ms();
    
    target* target_dummy = 0;
    target* target_snake = 0;
    target* target_agent = 0;
    target* target_minimake = 0;
    target* target_tests_run = 0;
    for (target* t = targetss.begin; (void*)t < targetss.end;) {
        const string target_dummy_name = STR("build_minimake/dummy");
        const string target_sname_name = STR("build_minimake/snake");
        const string target_agent_name = STR("build_minimake/agent");
        const string target_minimake_name = STR("build_minimake/minimake");
        const string tests_run_name = STR("build_minimake/tests_run");
        if (sa_equals(alloc, t->name.begin, t->name.end, target_dummy_name.begin, target_dummy_name.end)) {
            target_dummy = t;
        }
        if (sa_equals(alloc, t->name.begin, t->name.end, target_sname_name.begin, target_sname_name.end)) {
            target_snake = t;
        }
        if (sa_equals(alloc, t->name.begin, t->name.end, target_agent_name.begin, target_agent_name.end)) {
            target_agent = t;
        }
        if (sa_equals(alloc, t->name.begin, t->name.end, target_minimake_name.begin, target_minimake_name.end)) {
            target_minimake = t;
        }
        if (sa_equals(alloc, t->name.begin, t->name.end, tests_run_name.begin, tests_run_name.end)) {
            target_tests_run = t;
        }
        if (target_dummy && target_snake && target_agent && target_minimake && target_tests_run) {break;}
        t = t->end;
    }

    i32 return_code = 0;
    if (!target_build(targetss.begin, targetss.end, target_dummy, session, cache_dir, alloc)) {
        return_code = 1;
    }
    if (!target_build(targetss.begin, targetss.end, target_snake, session, cache_dir, alloc)) {
        return_code = 1;
    }
    if (!target_build(targetss.begin, targetss.end, target_agent, session, cache_dir, alloc)) {
        return_code = 1;
    }
    if (!target_build(targetss.begin, targetss.end, target_minimake, session, cache_dir, alloc)) {
        return_code = 1;
    }
    if (!target_build(targetss.begin, targetss.end, target_tests_run, session, cache_dir, alloc)) {
        return_code = 1;
    }

    u64 build_end_ms = sys_time_ms();
    print_format(file_stdout(), STRING("Setup took: %ums\n"), setup_end_ms - setup_begin_ms);
    print_format(file_stdout(), STRING("Targets took: %ums\n"), target_end_ms - target_begin_ms);
    print_format(file_stdout(), STRING("Builds took: %ums\n"), build_end_ms - build_begin_ms);
    
    sa_free(alloc, var_begin);
    close_persistent_shell(session);
    sa_free(alloc, session);
    
    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return return_code;
}
