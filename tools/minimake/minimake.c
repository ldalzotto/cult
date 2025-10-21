
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

typedef struct {target* begin; void* end;} targets;
typedef struct {target** begin; target** end;} target_refs;

static void push_target_ref(target* t, stack_alloc* alloc) {
    target** ref = sa_alloc(alloc, sizeof(*ref));
    *ref = t;
}

static void push_target_refs(targets ts, stack_alloc* alloc) {
    for (target* t = ts.begin; (void*)t < ts.end;) {
        push_target_ref(t, alloc);
        t = t->end;
    }
}

static targets create_c_object_targets(const string cc, const strings flags, string build_dir,
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
    return (targets){.begin = begin, alloc->cursor};
}

static target* create_executable_target(const string cc, const strings flags, string build_dir, string executable_file,
    target_refs deps,
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
    for (struct target** dep = deps.begin; dep < deps.end; ++dep) {
        push_string((*dep)->name, alloc);
    }

    finish_target(target, alloc);

    const uptr offset = bytesize(var_begin, var_end);
    sa_move_tail(alloc, var_end, var_begin);
    target_offset(begin, -offset);
    return begin;
}

// TODO: move to a zip build function
static u8 dummy(target* t, string cache_dir, stack_alloc* alloc) {
    string output_directory;
    directory_parent(t->name.begin, t->name.end, (void*)&output_directory.begin, (void*)&output_directory.end);
    directory_parent(output_directory.begin, output_directory.end, (void*)&output_directory.begin, (void*)&output_directory.end);
    directory_remove(alloc, output_directory.begin, output_directory.end);
    directory_create(alloc, output_directory.begin, output_directory.end, DIR_MODE_PUBLIC);

    string tar_gz_input_path = *t->deps;
    string marker_path = *(string*)t->deps->end;

    u8 success = 0;
    {
        void* begin = alloc->cursor;
        string command; command.begin = alloc->cursor;
        push_string_data(STRING("tar -xzvf "), alloc);
        push_string_data(tar_gz_input_path, alloc);
        push_string_data(STRING(" -C "), alloc);
        push_string_data(output_directory, alloc);
        command.end = alloc->cursor;

        print_format(file_stdout(), STRING("%s\n"), command);
        exec_command_result result = exec_command(command, alloc);
        string output = {result.output, alloc->cursor};
        print_format(file_stdout(), STRING("%s"), output);
        
        success = result.success;
        sa_free(alloc, begin);
    }

    // Create the marker
    if (success) {
        file_t file = file_open(alloc, marker_path.begin, marker_path.end, FILE_MODE_READ_WRITE);
        file_close(file);
    }

    if (success) {
        target_update_timestamp(t, cache_dir, alloc);
    }

    return success;
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

    strings window_c_files; window_c_files.begin = alloc->cursor;
    push_string(STRING("src/libs/window/win_x11.c"), alloc);
    window_c_files.end = alloc->cursor;

    strings window_c_flags; window_c_flags.begin = alloc->cursor;
    push_strings(common_c_flags, alloc);
    push_string(STRING("-I./src/elibs"), alloc);
    window_c_flags.end = alloc->cursor;

    strings x11_link_flags; x11_link_flags.begin = alloc->cursor;
    push_string(STRING("-lX11"), alloc);
    x11_link_flags.end = alloc->cursor;

    void* var_end = alloc->cursor;

    /* Create targets using helper functions */
    targets common_targets =
    create_c_object_targets(cc, common_c_flags, build_dir, common_c_files, alloc);

    targets coding_targets =
    create_c_object_targets(cc, coding_c_flags, build_dir, coding_c_files, alloc);

    targets network_targets =
    create_c_object_targets(cc, network_c_flags, build_dir, network_c_files, alloc);

    // X11 lib target
    target* x11_lib;
    {
        void* var_begin = alloc->cursor;

        string x11_tgz; x11_tgz.begin = alloc->cursor;
        push_string_data(STRING("elibs/x11_headers.tar.gz"), alloc);
        x11_tgz.end = alloc->cursor;

        string marker; marker.begin = alloc->cursor;
        push_string_data(build_dir, alloc);
        push_string_data(STRING("elibs/X11/.marker"), alloc);
        marker.end = alloc->cursor;

        void* var_end = alloc->cursor;
        
        target* t = create_target(alloc, STRING("build_minimake/elibs/X11/.timestamp"), dummy);
        t->template = alloc->cursor;
        t->deps = alloc->cursor;
        
        push_string(x11_tgz, alloc);
        push_string(marker, alloc);
        finish_target(t, alloc);

        const uptr offset = bytesize(var_begin, var_end);
        sa_move_tail(alloc, var_end, var_begin);
        x11_lib = var_begin;
        target_offset(var_begin, -offset);
    }

    targets window_targets =
    create_c_object_targets(cc, window_c_flags, build_dir, window_c_files, alloc);

    // TODO: window_targets that have x11_lib target as dep

    /* executable "foo" depends on foo.o and bar.o */
    {
        void* var_begin = alloc->cursor;

        strings link_flags; link_flags.begin = alloc->cursor;
        push_strings(network_link_flags, alloc);
        push_strings(x11_link_flags, alloc);
        link_flags.end = alloc->cursor;

        target_refs deps; deps.begin = alloc->cursor;
        push_target_refs(common_targets, alloc);
        push_target_refs(coding_targets, alloc);
        push_target_refs(network_targets, alloc);
        push_target_ref(x11_lib, alloc); // TODO: replace by window_targets when implemented
        push_target_refs(window_targets, alloc);
        deps.end = alloc->cursor;

        void* var_end = alloc->cursor;

        create_executable_target(cc, link_flags, build_dir, STRING("foo"), deps, alloc);
        
        sa_move_tail(alloc, var_end, var_begin);
        targets_offset(var_begin, -bytesize(var_begin, var_end), alloc);
    }

    sa_move_tail(alloc, var_end, var_begin);
    targets_offset(var_begin, -bytesize(var_begin, var_end), alloc);

    targetss.end = alloc->cursor;

    target* target_to_build = 0;
    for (target* t = targetss.begin; (void*)t < targetss.end;) {
        const string target_to_build_name = STR("build_minimake/foo");
        if (sa_equals(alloc, t->name.begin, t->name.end, target_to_build_name.begin, target_to_build_name.end)) {
            target_to_build = t;
            break;
        }
        t = t->end;
    }

    i32 return_code = 0;
    if (!target_build(targetss.begin, targetss.end, target_to_build, cache_dir, alloc)) {
        return_code = 1;
    }
    
    sa_free(alloc, memory);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return return_code;
}
