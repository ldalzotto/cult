
#include "mem.h"
#include "file.h"
#include "exec_command.h"
#include "system_time.h"
#include "print.h"
#include "minimake_script.h"

static targets make_targets(u8 use_debug, string build_dir, exec_command_session* session, stack_alloc* alloc) {
    targets targetss;
    targetss.begin = alloc->cursor;

    void* var_begin = alloc->cursor;

    const string cc = STR("gcc");

    // BEGIN - common
    strings common_c_flags = begin_strings(alloc);
    push_string(STRING("-Wall"), alloc);
    push_string(STRING("-Wextra"), alloc);
    push_string(STRING("-Werror"), alloc);
    push_string(STRING("-pedantic"), alloc);
    if (use_debug) {
        push_string(STRING("-DDEBUG_ASSERTIONS_ENABLED=1"), alloc);
        push_string(STRING("-Isrc/libs"), alloc);
    } else {
        // TODO
    }
    end_strings(&common_c_flags, alloc);

    strings common_link_flags = begin_strings(alloc);
    push_string(STRING("-no-pie"), alloc);
    end_strings(&common_link_flags, alloc);
    
    strings common_c_files = begin_strings(alloc);
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
    end_strings(&common_c_files, alloc);

    c_object_files common = make_c_object_files(common_c_files, build_dir, alloc);
    // END - common

    // BEGIN - coding
    strings coding_c_flags = begin_strings(alloc);
    push_strings(common_c_flags, alloc);
    end_strings(&coding_c_flags, alloc);
    
    strings coding_c_files = begin_strings(alloc);
    push_string(STRING("src/libs/coding/lz_deserialize.c"), alloc);
    push_string(STRING("src/libs/coding/lz_match_brute.c"), alloc);
    push_string(STRING("src/libs/coding/lz_serialize.c"), alloc);
    push_string(STRING("src/libs/coding/lz_window.c"), alloc);
    push_string(STRING("src/libs/coding/lzss.c"), alloc);
    end_strings(&coding_c_files, alloc);

    c_object_files coding = make_c_object_files(coding_c_files, build_dir, alloc);
    // END - coding

    // BEGIN - network
    strings network_link_flags = begin_strings(alloc);
    push_string(STRING("-lssl"), alloc);
    end_strings(&network_link_flags, alloc);

    strings network_c_flags = begin_strings(alloc);
    push_strings(common_c_flags, alloc);
    end_strings(&network_c_flags, alloc);

    strings network_c_files = begin_strings(alloc);
    push_string(STRING("src/libs/network/tcp/tcp_connection.c"), alloc);
    push_string(STRING("src/libs/network/tcp/tcp_read_write.c"), alloc);
    push_string(STRING("src/libs/network/https/https_request.c"), alloc);
    end_strings(&network_c_files, alloc);

    c_object_files network = make_c_object_files(network_c_files, build_dir, alloc);
    // END - network

    // BEGIN - x11
    u8 use_x11 = 0;
    {
        exec_command_result use_x11_result = command_session_exec_command(session, STRING("pkg-config --exists x11"), alloc);
        use_x11 = use_x11_result.success;
        sa_free(alloc, use_x11_result.output);
    }
    string x11_tgz = begin_string(alloc);
    push_string_data(STRING("elibs/x11_headers.tar.gz"), alloc);
    end_string(&x11_tgz, alloc);

    string x11_marker = begin_string(alloc);
    push_string_data(build_dir, alloc);
    push_string_data(STRING("elibs/X11/.marker"), alloc);
    end_string(&x11_marker, alloc);

    string x11_timestamp = begin_string(alloc);
    push_string_data(build_dir, alloc);
    push_string_data(STRING("elibs/X11/.timestamp"), alloc);
    end_string(&x11_timestamp, alloc);

    strings x11_link_flags = begin_strings(alloc);
    if (use_x11) {
        push_string(STRING("-lX11"), alloc);
    }
    end_strings(&x11_link_flags, alloc);
    // END - x11

    // BEGIN - window
    strings window_c_files = begin_strings(alloc);
    push_string(STRING("src/libs/window/win_x11.c"), alloc);
    if (!use_x11) {
        push_string(STRING("src/libs/window/x11_stub.c"), alloc);
    }
    end_strings(&window_c_files, alloc);

    c_object_files window = make_c_object_files(window_c_files, build_dir, alloc);

    strings window_c_flags = begin_strings(alloc);
    push_strings(common_c_flags, alloc);
    push_string(STRING("-I./src/elibs"), alloc);
    end_strings(&window_c_flags, alloc);

    strings window_deps = begin_strings(alloc);
    push_string(x11_timestamp, alloc);
    end_strings(&window_deps, alloc);
    // END - window

    // BEGIN - dummy
    strings dummy_c_files = begin_strings(alloc);
    push_string(STRING("src/apps/dummy/dummy.c"), alloc);
    end_strings(&dummy_c_files, alloc);

    c_object_files dummy = make_c_object_files(dummy_c_files, build_dir, alloc);

    strings dummy_c_flags = begin_strings(alloc);
    push_strings(common_c_flags, alloc);
    push_strings(window_c_flags, alloc);
    end_strings(&dummy_c_flags, alloc);

    strings dummy_link_flags = begin_strings(alloc);
    push_strings(common_link_flags, alloc);
    push_strings(x11_link_flags, alloc);
    end_strings(&dummy_link_flags, alloc);

    strings dummy_deps = begin_strings(alloc);
    push_strings(common.o, alloc);
    push_strings(window.o, alloc);
    push_strings(dummy.o, alloc);
    end_strings(&dummy_deps, alloc);

    string dummy_executable = make_c_executable_file(STRING("dummy"), build_dir, alloc);
    // END - dummy

    // BEGIN - snake
    strings snake_lib_c_files = begin_strings(alloc);
    push_string(STRING("src/apps/snake/snake_grid.c"), alloc);
    push_string(STRING("src/apps/snake/snake_move.c"), alloc);
    push_string(STRING("src/apps/snake/snake_render.c"), alloc);
    push_string(STRING("src/apps/snake/snake_reward.c"), alloc);
    push_string(STRING("src/apps/snake/snake.c"), alloc);
    end_strings(&snake_lib_c_files, alloc);

    c_object_files snake_lib = make_c_object_files(snake_lib_c_files, build_dir, alloc);

    strings snake_lib_c_flags = begin_strings(alloc);
    push_strings(common_c_flags, alloc);
    push_strings(window_c_flags, alloc);
    end_strings(&snake_lib_c_flags, alloc);

    strings snake_c_files = begin_strings(alloc);
    push_string(STRING("src/apps/snake/snake_loop.c"), alloc);
    end_strings(&snake_c_files, alloc);

    c_object_files snake = make_c_object_files(snake_c_files, build_dir, alloc);

    strings snake_c_flags = begin_strings(alloc);
    push_strings(snake_lib_c_flags, alloc);
    end_strings(&snake_c_flags, alloc);

    strings snake_link_flags = begin_strings(alloc);
    push_strings(common_link_flags, alloc);
    push_strings(x11_link_flags, alloc);
    end_strings(&snake_link_flags, alloc);

    strings snake_deps = begin_strings(alloc);
    push_strings(common.o, alloc);
    push_strings(window.o, alloc);
    push_strings(snake_lib.o, alloc);
    push_strings(snake.o, alloc);
    end_strings(&snake_deps, alloc);

    string snake_executable = make_c_executable_file(STRING("snake"), build_dir, alloc);
    // END - snake

    // BEGIN - agent
    strings agent_c_files = begin_strings(alloc);
    push_string(STRING("tools/agent/agent_args.c"), alloc);
    push_string(STRING("tools/agent/agent_request.c"), alloc);
    push_string(STRING("tools/agent/agent_result_write.c"), alloc);
    push_string(STRING("tools/agent/agent.c"), alloc);
    push_string(STRING("tools/agent/user_content_read.c"), alloc);
    end_strings(&agent_c_files, alloc);

    c_object_files agent = make_c_object_files(agent_c_files, build_dir, alloc);

    strings agent_c_flags = begin_strings(alloc);
    push_strings(common_c_flags, alloc);
    end_strings(&agent_c_flags, alloc);

    strings agent_link_flags = begin_strings(alloc);
    push_strings(common_link_flags, alloc);
    push_strings(network_link_flags, alloc);
    end_strings(&agent_link_flags, alloc);

    strings agent_deps = begin_strings(alloc);
    push_strings(common.o, alloc);
    push_strings(network.o, alloc);
    push_strings(agent.o, alloc);
    end_strings(&agent_deps, alloc);

    string agent_executable = make_c_executable_file(STRING("agent"), build_dir, alloc);
    // END - agent

    // BEGIN - minimake
    strings minimake_c_files = begin_strings(alloc);
    push_string(STRING("tools/minimake/minimake_script.c"), alloc);
    push_string(STRING("tools/minimake/minimake.c"), alloc);
    push_string(STRING("tools/minimake/target_build.c"), alloc);
    push_string(STRING("tools/minimake/target_c_dependencies.c"), alloc);
    push_string(STRING("tools/minimake/target_execution_list.c"), alloc);
    push_string(STRING("tools/minimake/target_timestamp.c"), alloc);
    end_strings(&minimake_c_files, alloc);

    c_object_files minimake = make_c_object_files(minimake_c_files, build_dir, alloc);

    strings minimake_c_flags = begin_strings(alloc);
    push_strings(common_c_flags, alloc);
    end_strings(&minimake_c_flags, alloc);

    strings minimake_link_flags = begin_strings(alloc);
    push_strings(common_link_flags, alloc);
    end_strings(&minimake_link_flags, alloc);

    strings minimake_deps = begin_strings(alloc);
    push_strings(common.o, alloc);
    push_strings(minimake.o, alloc);
    end_strings(&minimake_deps, alloc);

    string minimake_executable = make_c_executable_file(STRING("minimake"), build_dir, alloc);
    // END - minimake

    // BEGIN - tests
    strings tests_c_files = begin_strings(alloc);
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
    end_strings(&tests_c_files, alloc);

    c_object_files tests = make_c_object_files(tests_c_files, build_dir, alloc);

    strings tests_c_flags = begin_strings(alloc);
    push_strings(common_c_flags, alloc);
    push_string(STRING("-Isrc/apps"), alloc);
    end_strings(&tests_c_flags, alloc);

    strings tests_link_flags = begin_strings(alloc);
    push_strings(common_link_flags, alloc);
    push_strings(x11_link_flags, alloc);
    push_strings(network_link_flags, alloc);
    end_strings(&tests_link_flags, alloc);

    strings tests_deps = begin_strings(alloc);
    push_strings(common.o, alloc);
    push_strings(window.o, alloc);
    push_strings(coding.o, alloc);
    push_strings(network.o, alloc);
    push_strings(snake_lib.o, alloc);
    push_strings(tests.o, alloc);
    end_strings(&tests_deps, alloc);

    string tests_executblabe = make_c_executable_file(STRING("tests_run"), build_dir, alloc);
    // END - tests

    // BEGIN - make all
    strings make_all_deps = begin_strings(alloc);
    push_string(dummy_executable, alloc);
    push_string(snake_executable, alloc);
    push_string(agent_executable, alloc);
    push_string(minimake_executable, alloc);
    push_string(tests_executblabe, alloc);
    end_strings(&make_all_deps, alloc);
    // END - make all

    void* var_end = alloc->cursor;
    
    create_c_object_targets(cc, common_c_flags, common, (strings){0,0}, alloc);
    create_c_object_targets(cc, coding_c_flags, coding, (strings){0,0}, alloc);
    create_c_object_targets(cc, network_c_flags, network, (strings){0,0}, alloc);

    // X11 lib target
    create_extract_target(x11_tgz, x11_marker, x11_timestamp, alloc);

    create_c_object_targets(cc, window_c_flags, window, window_deps, alloc);
    
    create_c_object_targets(cc, dummy_c_flags, dummy, (strings){0,0}, alloc);
    create_executable_target(cc, dummy_link_flags, dummy_executable, dummy_deps, alloc);

    create_c_object_targets(cc, snake_lib_c_flags, snake_lib, (strings){0,0}, alloc);
    create_c_object_targets(cc, snake_c_flags, snake, (strings){0,0}, alloc);
    create_executable_target(cc, snake_link_flags, snake_executable, snake_deps, alloc);

    create_c_object_targets(cc, agent_c_flags, agent, (strings){0,0}, alloc);
    create_executable_target(cc, agent_link_flags, agent_executable, agent_deps, alloc);

    create_c_object_targets(cc, minimake_c_flags, minimake, (strings){0,0}, alloc);
    create_executable_target(cc, minimake_link_flags, minimake_executable, minimake_deps, alloc);

    create_c_object_targets(cc, tests_c_flags, tests, (strings){0,0}, alloc);
    create_executable_target(cc, tests_link_flags, tests_executblabe, tests_deps, alloc);

    create_phony_target(STRING("all"), make_all_deps, build_dir, alloc);

    sa_move_tail(alloc, var_end, var_begin);
    targets_offset(var_begin, -bytesize(var_begin, var_end), alloc);

    targetss.end = alloc->cursor;

    return targetss;
}

i32 main(i32 argc, char** argv) {
    const uptr memory_size = 1024 * 1024;
    void* memory = mem_map(memory_size);

    stack_alloc _alloc;
    stack_alloc* alloc = &_alloc;
    sa_init(alloc, memory, (char*)memory + memory_size);

    exec_command_session* session = open_persistent_shell(alloc);

    const string build_dir = STR("build_minimake/");
    const string cache_dir = STR("build_minimake/.minimake/");

    directory_create(alloc, build_dir.begin, build_dir.end, DIR_MODE_PUBLIC);
    directory_create(alloc, cache_dir.begin, cache_dir.end, DIR_MODE_PUBLIC);

    u64 target_begin_ms = sys_time_ms();

    targets targetss = make_targets(/*use_debug=*/ 1, build_dir, session, alloc);

    u64 target_end_ms = sys_time_ms();

    u64 build_begin_ms = sys_time_ms();

    /* Get the target name(s) from the command line instead of hardcoding.
       If no target is provided, build the default set. */
    u8 return_code = 1;
    if (argc >= 2) {
        /* Build each target passed on the command line (argv[1] ... argv[argc-1]). */
        for (i32 i = 1; i < argc; ++i) {
            string s;
            s.begin = (u8*)argv[i];
            s.end = byteoffset(argv[i], mem_cstrlen((void*)argv[i]));
            return_code = return_code & target_build_name(targetss, s, build_dir, cache_dir, session, alloc);
        }
    }

    u64 build_end_ms = sys_time_ms();
    print_format(file_stdout(), STRING("Targets took: %ums\n"), target_end_ms - target_begin_ms);
    print_format(file_stdout(), STRING("Builds took: %ums\n"), build_end_ms - build_begin_ms);
    
    sa_free(alloc, targetss.begin);
    close_persistent_shell(session);
    sa_free(alloc, session);
    
    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return return_code ? 0 : 1;
}
