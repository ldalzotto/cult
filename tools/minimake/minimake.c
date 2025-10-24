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

        // print_format(file_stdout(), STRING("%s\n"), command);
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

typedef struct { string* cc; string* cfile; strings c_flags; } target_local_object;
typedef struct { string* tar; string* marker; } target_local_extract;
typedef struct { string* cc; strings link_flags; } target_local_executable;
typedef enum { target_local_kind_object, target_local_kind_extract, target_local_kind_executable, } target_local_kind;
typedef struct { string* name; strings deps; target_local_kind kind; void* external; void* end; } target_local;
typedef struct { target_local* begin; void* end; } targets_local;

static void target_local_offset(target_local* t, uptr offset) {
    t->name = byteoffset(t->name, offset);
    t->name->begin = byteoffset(t->name->begin, offset);
    t->name->end = byteoffset(t->name->end, offset);
    t->deps.begin = byteoffset(t->deps.begin, offset);
    t->deps.end = byteoffset(t->deps.end, offset);
    t->external = byteoffset(t->external, offset);
    switch (t->kind) {
        case target_local_kind_object: {
            target_local_object* object = t->external;
            object->cc = byteoffset(object->cc, offset);
            object->cc->begin = byteoffset(object->cc->begin, offset);
            object->cc->end = byteoffset(object->cc->end, offset);
            object->cfile = byteoffset(object->cfile, offset);
            object->cfile->begin = byteoffset(object->cfile->begin, offset);
            object->cfile->end = byteoffset(object->cfile->end, offset);
            object->c_flags.begin = byteoffset(object->c_flags.begin, offset);
            object->c_flags.end = byteoffset(object->c_flags.end, offset);
            for (string* f = object->c_flags.begin; (void*)f<object->c_flags.end;) {
                f->begin = byteoffset(f->begin, offset);
                f->end = byteoffset(f->end, offset);
                f=(void*)f->end;
            }
        } break;
        case target_local_kind_extract: {
            target_local_extract* extract = t->external;
            extract->tar = byteoffset(extract->tar, offset);
            extract->tar->begin = byteoffset(extract->tar->begin, offset);
            extract->tar->end = byteoffset(extract->tar->end, offset);
            extract->marker = byteoffset(extract->marker, offset);
            extract->marker->begin = byteoffset(extract->marker->begin, offset);
            extract->marker->end = byteoffset(extract->marker->end, offset);
        } break;
        case target_local_kind_executable: {
            target_local_executable* executable = t->external;
            executable->cc = byteoffset(executable->cc, offset);
            executable->cc->begin = byteoffset(executable->cc->begin, offset);
            executable->cc->end = byteoffset(executable->cc->end, offset);
            executable->link_flags.begin = byteoffset(executable->link_flags.begin, offset);
            executable->link_flags.end = byteoffset(executable->link_flags.end, offset);
            for (string* f = executable->link_flags.begin; (void*)f<executable->link_flags.end;) {
                f->begin = byteoffset(f->begin, offset);
                f->end = byteoffset(f->end, offset);
                f=(void*)f->end;
            }
        } break;
    }
    for (string* d = t->deps.begin; (void*)d < t->deps.end; ) {
        d->begin = byteoffset(d->begin, offset);
        d->end = byteoffset(d->end, offset);
        d = (void*)d->end;
    }
    t->end = byteoffset(t->end, offset);
}

static void targets_local_offset(target_local* begin, void* end, uptr offset) {
    for (target_local* t = begin; (void*)t < end;) {
        target_local_offset(t, offset);
        t = t->end;
    }
}

static void target_local_object_set_name(stack_alloc* alloc, target_local* l, string build_dir, string c_file) {
    l->name = sa_alloc(alloc, sizeof(*l->name));
    l->name->begin = alloc->cursor;
    push_string_data(build_dir, alloc);
    push_string_data(c_file, alloc);
    l->name->end = alloc->cursor;
    *((u8*)l->name->end - 1) = 'o';
}

static target_local* minimake_main(stack_alloc* alloc, string build_dir) {
    unused(build_dir);
    void* begin = alloc->cursor;

    void* var_begin = alloc->cursor;

    const string cc = STR("gcc");
    strings common_c_flags;
    common_c_flags.begin = alloc->cursor;
    push_string(STRING("-Wall"), alloc);
    push_string(STRING("-Wextra"), alloc);
    push_string(STRING("-Werror"), alloc);
    push_string(STRING("-pedantic"), alloc);
    // -O3 -DNDEBUG -DDEBUG_ASSERTIONS_ENABLED=0
    const u8 is_debug = 1;
    if (is_debug) {
    push_string(STRING("-O0"), alloc);
    push_string(STRING("-g"), alloc);
    push_string(STRING("-DDEBUG_ASSERTIONS_ENABLED=1"), alloc);
    } else {
    push_string(STRING("-O3"), alloc);
    push_string(STRING("-DNDEBUG"), alloc);
    push_string(STRING("-DDEBUG_ASSERTIONS_ENABLED=0"), alloc);
    }

    push_string(STRING("-Isrc/libs"), alloc);
    common_c_flags.end = alloc->cursor;

    strings common_link_flags; common_link_flags.begin = alloc->cursor;
    push_string(STRING("-no-pie"), alloc);
    common_link_flags.end = alloc->cursor;

    u8 use_x11 = 0;
    {
        exec_command_result use_x11_result = exec_command(STRING("pkg-config --exists x11"), alloc);
        use_x11 = use_x11_result.success;
        sa_free(alloc, use_x11_result.output);
    }


    void* var_end = alloc->cursor;

    {
        void* var_begin = alloc->cursor;
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

        void* var_end = alloc->cursor;

        for (string* s = common_c_files.begin; (void*)s < common_c_files.end;) {
            target_local* l = sa_alloc(alloc, sizeof(*l));
            target_local_object_set_name(alloc, l, build_dir, *s);
            l->deps.begin = alloc->cursor; l->deps.end = alloc->cursor;
            l->kind = target_local_kind_object;
            target_local_object* object = sa_alloc(alloc, sizeof(*(object)));
            l->external = object;
            
            object->cc = alloc->cursor; push_string(cc, alloc);
            object->cfile = alloc->cursor; push_string(*s, alloc);
            object->c_flags.begin = alloc->cursor;
            push_strings(common_c_flags, alloc);
            object->c_flags.end = alloc->cursor;

            l->end = alloc->cursor;
            s = (void*)s->end;
        }

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

    {
        void* var_begin = alloc->cursor;
        
        strings coding_c_files;
        coding_c_files.begin = alloc->cursor;

        push_string(STRING("src/libs/coding/lz_deserialize.c"), alloc);
        push_string(STRING("src/libs/coding/lz_match_brute.c"), alloc);
        push_string(STRING("src/libs/coding/lz_serialize.c"), alloc);
        push_string(STRING("src/libs/coding/lz_window.c"), alloc);
        push_string(STRING("src/libs/coding/lzss.c"), alloc);

        coding_c_files.end = alloc->cursor;
        
        void* var_end = alloc->cursor;

        for (string* s = coding_c_files.begin; (void*)s < coding_c_files.end;) {
            target_local* l = sa_alloc(alloc, sizeof(*l));
            target_local_object_set_name(alloc, l, build_dir, *s);
            l->deps.begin = alloc->cursor; l->deps.end = alloc->cursor;
            l->kind = target_local_kind_object;
            target_local_object* object = sa_alloc(alloc, sizeof(*(object)));
            l->external = object;
            
            object->cc = alloc->cursor; push_string(cc, alloc);
            object->cfile = alloc->cursor; push_string(*s, alloc);
            object->c_flags.begin = alloc->cursor;
            push_strings(common_c_flags, alloc);
            object->c_flags.end = alloc->cursor;

            l->end = alloc->cursor;
            s = (void*)s->end;
        }

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

    {
        void* var_begin = alloc->cursor;
        
        strings network_c_files;
        network_c_files.begin = alloc->cursor;

        push_string(STRING("src/libs/network/tcp/tcp_connection.c"), alloc);
        push_string(STRING("src/libs/network/tcp/tcp_read_write.c"), alloc);
        push_string(STRING("src/libs/network/https/https_request.c"), alloc);

        network_c_files.end = alloc->cursor;
        
        void* var_end = alloc->cursor;

        for (string* s = network_c_files.begin; (void*)s < network_c_files.end;) {
            target_local* l = sa_alloc(alloc, sizeof(*l));
            target_local_object_set_name(alloc, l, build_dir, *s);
            l->deps.begin = alloc->cursor; l->deps.end = alloc->cursor;
            l->kind = target_local_kind_object;
            target_local_object* object = sa_alloc(alloc, sizeof(*(object)));
            l->external = object;
            
            object->cc = alloc->cursor; push_string(cc, alloc);
            object->cfile = alloc->cursor; push_string(*s, alloc);
            object->c_flags.begin = alloc->cursor;
            push_strings(common_c_flags, alloc);
            object->c_flags.end = alloc->cursor;

            l->end = alloc->cursor;
            s = (void*)s->end;
        }

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

    {
        target_local* l = sa_alloc(alloc, sizeof(*l));
        l->name = sa_alloc(alloc, sizeof(*l->name));
        l->name->begin = alloc->cursor;
        push_string_data(build_dir, alloc);
        push_string_data(STRING("elibs/X11/.timestamp"), alloc);
        l->name->end = alloc->cursor;
        l->deps = (strings){alloc->cursor, alloc->cursor};
        l->kind = target_local_kind_extract;
        target_local_extract* e = sa_alloc(alloc, sizeof(*e));
        l->external = e;
        e->tar = alloc->cursor;
        push_string(STRING("elibs/x11_headers.tar.gz"), alloc);
        e->marker = alloc->cursor;
        push_string(STRING("elibs/X11/.marker"), alloc);
        l->end = alloc->cursor;
    }

    {
        void* var_begin = alloc->cursor;
        
        strings window_c_files;
        window_c_files.begin = alloc->cursor;

        push_string(STRING("src/libs/window/win_x11.c"), alloc);
        push_string(STRING("src/libs/window/x11_stub.c"), alloc);
        
        window_c_files.end = alloc->cursor;

        strings window_c_flags;
        window_c_flags.begin = alloc->cursor;
        push_strings(common_c_flags, alloc);
        push_string(STRING("-I./src/elibs"), alloc);
        window_c_flags.end = alloc->cursor;
        
        void* var_end = alloc->cursor;

        for (string* s = window_c_files.begin; (void*)s < window_c_files.end;) {
            target_local* l = sa_alloc(alloc, sizeof(*l));
            target_local_object_set_name(alloc, l, build_dir, *s);
            l->deps.begin = alloc->cursor;
            {
                string* d = sa_alloc(alloc, sizeof(*d));
                d->begin = alloc->cursor;
                push_string_data(build_dir, alloc);
                push_string_data(STRING("elibs/X11/.timestamp"), alloc);
                d->end = alloc->cursor;
            }
            
            l->deps.end = alloc->cursor;
            l->kind = target_local_kind_object;
            target_local_object* object = sa_alloc(alloc, sizeof(*(object)));
            l->external = object;
            
            object->cc = alloc->cursor; push_string(cc, alloc);
            object->cfile = alloc->cursor; push_string(*s, alloc);
            object->c_flags.begin = alloc->cursor;
            push_strings(window_c_flags, alloc);
            object->c_flags.end = alloc->cursor;

            l->end = alloc->cursor;
            s = (void*)s->end;
        }

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

    {
        void* var_begin = alloc->cursor;

        strings dummy_c_files; dummy_c_files.begin = alloc->cursor;
        push_string(STRING("src/apps/dummy/dummy.c"), alloc);
        dummy_c_files.end = alloc->cursor;

        void* var_end = alloc->cursor;

        for (string* s = dummy_c_files.begin; (void*)s < dummy_c_files.end;) {
            target_local* l = sa_alloc(alloc, sizeof(*l));
            target_local_object_set_name(alloc, l, build_dir, *s);
            l->deps = (strings){alloc->cursor, alloc->cursor};
            l->kind = target_local_kind_object;
            target_local_object* o = sa_alloc(alloc, sizeof(*o));
            l->external = o;
            o->cc = alloc->cursor;
            push_string(cc, alloc);
            o->cfile = alloc->cursor; push_string(*s, alloc);
            o->c_flags.begin = alloc->cursor;
            push_strings(common_c_flags, alloc);
            o->c_flags.end = alloc->cursor;
            l->end = alloc->cursor;
            s = (void*)s->end;
        }

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

    {
        void* var_begin = alloc->cursor;

        strings dummy_objects; dummy_objects.begin = alloc->cursor;
        push_string(STRING("src/libs/assert.o"), alloc);
        push_string(STRING("src/libs/backtrace.o"), alloc);
        push_string(STRING("src/libs/convert.o"), alloc);
        push_string(STRING("src/libs/exec_command.o"), alloc);
        push_string(STRING("src/libs/file.o"), alloc);
        push_string(STRING("src/libs/format_iterator.o"), alloc);
        push_string(STRING("src/libs/mem.o"), alloc);
        push_string(STRING("src/libs/meta_iterator.o"), alloc);
        push_string(STRING("src/libs/print.o"), alloc);
        push_string(STRING("src/libs/stack_alloc.o"), alloc);
        push_string(STRING("src/libs/system_time.o"), alloc);
        push_string(STRING("src/libs/time.o"), alloc);

        push_string(STRING("src/libs/window/win_x11.o"), alloc);
        if (!use_x11) {
            push_string(STRING("src/libs/window/x11_stub.o"), alloc);
        }

        push_string(STRING("src/apps/dummy/dummy.o"), alloc);
        dummy_objects.end = alloc->cursor;

        strings dummy_link_flags; dummy_link_flags.begin =alloc->cursor;
        if (use_x11) {
            push_string(STRING("-lX11"), alloc);
        }
        push_strings(common_link_flags, alloc);
        dummy_link_flags.end =alloc->cursor;

        void* var_end = alloc->cursor;

        target_local* l = sa_alloc(alloc, sizeof(*l));
        l->name = sa_alloc(alloc, sizeof(*l->name));
        l->name->begin = alloc->cursor;
        push_string_data(build_dir, alloc);
        push_string_data(STRING("dummy"), alloc);
        l->name->end = alloc->cursor;
        
        l->deps.begin = alloc->cursor;

        for (string* dummy_object = dummy_objects.begin; (void*)dummy_object < dummy_objects.end;) {
            string* s = sa_alloc(alloc, sizeof(*s));
            s->begin = alloc->cursor;
            push_string_data(build_dir, alloc);
            push_string_data(*dummy_object, alloc);
            s->end = alloc->cursor;
            dummy_object = (void*)dummy_object->end;
        }
        
        l->deps.end = alloc->cursor;
        l->kind = target_local_kind_executable;
        target_local_executable* e = sa_alloc(alloc, sizeof(*e));
        l->external = e;
        e->cc = alloc->cursor;
        push_string(cc, alloc);
        e->link_flags.begin = alloc->cursor;
        push_strings(dummy_link_flags, alloc);
        e->link_flags.end = alloc->cursor;
        l->end = alloc->cursor;

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

     {
        void* var_begin = alloc->cursor;

        strings snake_c_files; snake_c_files.begin = alloc->cursor;
        push_string(STRING("src/apps/snake/snake_grid.c"), alloc);
        push_string(STRING("src/apps/snake/snake_loop.c"), alloc);
        push_string(STRING("src/apps/snake/snake_move.c"), alloc);
        push_string(STRING("src/apps/snake/snake_render.c"), alloc);
        push_string(STRING("src/apps/snake/snake_reward.c"), alloc);
        push_string(STRING("src/apps/snake/snake.c"), alloc);
        snake_c_files.end = alloc->cursor;

        void* var_end = alloc->cursor;

        for (string* s = snake_c_files.begin; (void*)s < snake_c_files.end;) {
            target_local* l = sa_alloc(alloc, sizeof(*l));
            target_local_object_set_name(alloc, l, build_dir, *s);
            l->deps = (strings){alloc->cursor, alloc->cursor};
            l->kind = target_local_kind_object;
            target_local_object* o = sa_alloc(alloc, sizeof(*o));
            l->external = o;
            o->cc = alloc->cursor;
            push_string(cc, alloc);
            o->cfile = alloc->cursor; push_string(*s, alloc);
            o->c_flags.begin = alloc->cursor;
            push_strings(common_c_flags, alloc);
            o->c_flags.end = alloc->cursor;
            l->end = alloc->cursor;
            s = (void*)s->end;
        }

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

    {
        void* var_begin = alloc->cursor;

        strings snake_objects; snake_objects.begin = alloc->cursor;
        push_string(STRING("src/libs/assert.o"), alloc);
        push_string(STRING("src/libs/backtrace.o"), alloc);
        push_string(STRING("src/libs/convert.o"), alloc);
        push_string(STRING("src/libs/exec_command.o"), alloc);
        push_string(STRING("src/libs/file.o"), alloc);
        push_string(STRING("src/libs/format_iterator.o"), alloc);
        push_string(STRING("src/libs/mem.o"), alloc);
        push_string(STRING("src/libs/meta_iterator.o"), alloc);
        push_string(STRING("src/libs/print.o"), alloc);
        push_string(STRING("src/libs/stack_alloc.o"), alloc);
        push_string(STRING("src/libs/system_time.o"), alloc);
        push_string(STRING("src/libs/time.o"), alloc);

        push_string(STRING("src/libs/window/win_x11.o"), alloc);
        if (!use_x11) {
            push_string(STRING("src/libs/window/x11_stub.o"), alloc);
        }

        push_string(STRING("src/apps/snake/snake_grid.o"), alloc);
        push_string(STRING("src/apps/snake/snake_loop.o"), alloc);
        push_string(STRING("src/apps/snake/snake_move.o"), alloc);
        push_string(STRING("src/apps/snake/snake_render.o"), alloc);
        push_string(STRING("src/apps/snake/snake_reward.o"), alloc);
        push_string(STRING("src/apps/snake/snake.o"), alloc);
        snake_objects.end = alloc->cursor;

        strings snake_link_flags; snake_link_flags.begin =alloc->cursor;
        if (use_x11) {
            push_string(STRING("-lX11"), alloc);
        }
        push_strings(common_link_flags, alloc);
        snake_link_flags.end =alloc->cursor;

        void* var_end = alloc->cursor;

        target_local* l = sa_alloc(alloc, sizeof(*l));
        l->name = sa_alloc(alloc, sizeof(*l->name));
        l->name->begin = alloc->cursor;
        push_string_data(build_dir, alloc);
        push_string_data(STRING("snake"), alloc);
        l->name->end = alloc->cursor;
        
        l->deps.begin = alloc->cursor;

        for (string* snake_object = snake_objects.begin; (void*)snake_object < snake_objects.end;) {
            string* s = sa_alloc(alloc, sizeof(*s));
            s->begin = alloc->cursor;
            push_string_data(build_dir, alloc);
            push_string_data(*snake_object, alloc);
            s->end = alloc->cursor;
            snake_object = (void*)snake_object->end;
        }
        
        l->deps.end = alloc->cursor;
        l->kind = target_local_kind_executable;
        target_local_executable* e = sa_alloc(alloc, sizeof(*e));
        l->external = e;
        e->cc = alloc->cursor;
        push_string(cc, alloc);
        e->link_flags.begin = alloc->cursor;
        push_strings(snake_link_flags, alloc);
        e->link_flags.end = alloc->cursor;
        l->end = alloc->cursor;

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

    {
        void* var_begin = alloc->cursor;

        strings test_c_files; test_c_files.begin = alloc->cursor;
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
        test_c_files.end = alloc->cursor;

        strings test_c_flags; test_c_flags.begin = alloc->cursor;
        push_strings(common_c_flags, alloc);
        push_string(STRING("-Isrc/apps"), alloc);
        test_c_flags.end = alloc->cursor;

        void* var_end = alloc->cursor;

        for (string* s = test_c_files.begin; (void*)s < test_c_files.end;) {
            target_local* l = sa_alloc(alloc, sizeof(*l));
            target_local_object_set_name(alloc, l, build_dir, *s);
            l->deps = (strings){alloc->cursor, alloc->cursor};
            l->kind = target_local_kind_object;
            target_local_object* o = sa_alloc(alloc, sizeof(*o));
            l->external = o;
            o->cc = alloc->cursor;
            push_string(cc, alloc);
            o->cfile = alloc->cursor; push_string(*s, alloc);
            o->c_flags.begin = alloc->cursor;
            push_strings(test_c_flags, alloc);
            o->c_flags.end = alloc->cursor;
            l->end = alloc->cursor;
            s = (void*)s->end;
        }

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

    {
        void* var_begin = alloc->cursor;

        strings test_objects; test_objects.begin = alloc->cursor;
        push_string(STRING("src/libs/assert.o"), alloc);
        push_string(STRING("src/libs/backtrace.o"), alloc);
        push_string(STRING("src/libs/convert.o"), alloc);
        push_string(STRING("src/libs/exec_command.o"), alloc);
        push_string(STRING("src/libs/file.o"), alloc);
        push_string(STRING("src/libs/format_iterator.o"), alloc);
        push_string(STRING("src/libs/mem.o"), alloc);
        push_string(STRING("src/libs/meta_iterator.o"), alloc);
        push_string(STRING("src/libs/print.o"), alloc);
        push_string(STRING("src/libs/stack_alloc.o"), alloc);
        push_string(STRING("src/libs/system_time.o"), alloc);
        push_string(STRING("src/libs/time.o"), alloc);

        push_string(STRING("src/libs/coding/lz_deserialize.o"), alloc);
        push_string(STRING("src/libs/coding/lz_match_brute.o"), alloc);
        push_string(STRING("src/libs/coding/lz_serialize.o"), alloc);
        push_string(STRING("src/libs/coding/lz_window.o"), alloc);
        push_string(STRING("src/libs/coding/lzss.o"), alloc);

        push_string(STRING("src/libs/window/win_x11.o"), alloc);
        if (!use_x11) {
            push_string(STRING("src/libs/window/x11_stub.o"), alloc);
        }

        push_string(STRING("src/libs/network/tcp/tcp_connection.o"), alloc);
        push_string(STRING("src/libs/network/tcp/tcp_read_write.o"), alloc);
        push_string(STRING("src/libs/network/https/https_request.o"), alloc);

        push_string(STRING("src/apps/snake/snake_grid.o"), alloc);
        push_string(STRING("src/apps/snake/snake_move.o"), alloc);
        push_string(STRING("src/apps/snake/snake_render.o"), alloc);
        push_string(STRING("src/apps/snake/snake_reward.o"), alloc);
        push_string(STRING("src/apps/snake/snake.o"), alloc);


        push_string(STRING("tests/all_tests.o"), alloc);
        push_string(STRING("tests/test_backtrace.o"), alloc);
        push_string(STRING("tests/test_exec_command.o"), alloc);
        push_string(STRING("tests/test_file.o"), alloc);
        push_string(STRING("tests/test_fps_ticker.o"), alloc);
        push_string(STRING("tests/test_framework.o"), alloc);
        push_string(STRING("tests/test_lzss.o"), alloc);
        push_string(STRING("tests/test_mem.o"), alloc);
        push_string(STRING("tests/test_network_https.o"), alloc);
        push_string(STRING("tests/test_network_tcp.o"), alloc);
        push_string(STRING("tests/test_print.o"), alloc);
        push_string(STRING("tests/test_snake.o"), alloc);
        push_string(STRING("tests/test_stack_alloc.o"), alloc);
        push_string(STRING("tests/test_temp_dir.o"), alloc);
        push_string(STRING("tests/test_win_x11.o"), alloc);

        test_objects.end = alloc->cursor;

        strings test_link_flags; test_link_flags.begin =alloc->cursor;
        if (use_x11) {
            push_string(STRING("-lX11"), alloc);
        }
        push_string(STRING("-lssl"), alloc);
        push_strings(common_link_flags, alloc);
        test_link_flags.end =alloc->cursor;

        void* var_end = alloc->cursor;

        target_local* l = sa_alloc(alloc, sizeof(*l));
        l->name = sa_alloc(alloc, sizeof(*l->name));
        l->name->begin = alloc->cursor;
        push_string_data(build_dir, alloc);
        push_string_data(STRING("run_tests"), alloc);
        l->name->end = alloc->cursor;
        
        l->deps.begin = alloc->cursor;

        for (string* test_object = test_objects.begin; (void*)test_object < test_objects.end;) {
            string* s = sa_alloc(alloc, sizeof(*s));
            s->begin = alloc->cursor;
            push_string_data(build_dir, alloc);
            push_string_data(*test_object, alloc);
            s->end = alloc->cursor;
            test_object = (void*)test_object->end;
        }
        
        l->deps.end = alloc->cursor;
        l->kind = target_local_kind_executable;
        target_local_executable* e = sa_alloc(alloc, sizeof(*e));
        l->external = e;
        e->cc = alloc->cursor;
        push_string(cc, alloc);
        e->link_flags.begin = alloc->cursor;
        push_strings(test_link_flags, alloc);
        e->link_flags.end = alloc->cursor;
        l->end = alloc->cursor;

        const uptr var_size = bytesize(var_begin, var_end);
        void* end = byteoffset(alloc->cursor, -var_size);
        sa_move_tail(alloc, var_end, var_begin);
        targets_local_offset(var_begin, end, -var_size);
    }

    const uptr var_size = bytesize(var_begin, var_end);
    void* end = byteoffset(alloc->cursor, -var_size);
    sa_move_tail(alloc, var_end, var_begin);
    targets_local_offset(var_begin, end, -var_size);

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

    targets_local targets_local;
    targets_local.begin = minimake_main(alloc, build_dir);
    targets_local.end = alloc->cursor;

    for (target_local* tl = targets_local.begin; (void*)tl < targets_local.end;) {
        switch (tl->kind) {
        case target_local_kind_object: {
            target* t = sa_alloc(alloc, sizeof(*t));
            target_local_object* object = tl->external;
            t->name.begin = alloc->cursor;
            push_string_data(*tl->name, alloc);
            t->name.end = alloc->cursor;
            t->build = &target_build_object;
            t->template = alloc->cursor;
            make_build_object_template(alloc, *object->cc, object->c_flags);
            t->deps = alloc->cursor;
            for (string* d = tl->deps.begin; (void*)d < tl->deps.end;) {
                string* s = sa_alloc(alloc, sizeof(*s));
                s->begin = alloc->cursor;
                push_string_data(*d, alloc);
                s->end = alloc->cursor;
                d = (void*)d->end;
            }
            // TODO: we can ditch the template and update extract_c_dependencies instead. That would simplify the program.
            {
                void* var_begin = alloc->cursor;
                string template;
                template.begin = alloc->cursor;
                make_extract_dependency_template(alloc, *object->cc, object->c_flags);
                template.end = alloc->cursor;
                void* var_end = alloc->cursor;

                strings dynamic_deps;
                dynamic_deps.begin = extract_c_dependencies(*object->cfile, template, alloc);
                dynamic_deps.end = alloc->cursor;

                sa_move_tail(alloc, var_end, var_begin);
                const uptr offset = -bytesize(var_begin, var_end);
                dynamic_deps.begin = byteoffset(dynamic_deps.begin, offset);
                dynamic_deps.end = byteoffset(dynamic_deps.end, offset);
                for (string* dynamic_dep = dynamic_deps.begin; (void*)dynamic_dep < dynamic_deps.end;) {
                    dynamic_dep->begin = byteoffset(dynamic_dep->begin, offset);
                    dynamic_dep->end = byteoffset(dynamic_dep->end, offset);
                    dynamic_dep = (void*)dynamic_dep->end;
                }
            }
            t->end = alloc->cursor;
        } break;
        case target_local_kind_executable: {
            target* t = sa_alloc(alloc, sizeof(*t));
            target_local_executable* executable = tl->external;
            t->name.begin = alloc->cursor;
            push_string_data(*tl->name, alloc);
            t->name.end = alloc->cursor;
            t->build = &target_build_executable;
            t->template = alloc->cursor;
            make_executable_link_template(alloc, *executable->cc, executable->link_flags);
            t->deps = alloc->cursor;
            for (string* d = tl->deps.begin; (void*)d < tl->deps.end;) {
                string* s = sa_alloc(alloc, sizeof(*s));
                s->begin = alloc->cursor;
                push_string_data(*d, alloc);
                s->end = alloc->cursor;
                d = (void*)d->end;
            }
            t->end = alloc->cursor;
        } break;
        case target_local_kind_extract: {
            target* t = sa_alloc(alloc, sizeof(*t));
            target_local_extract* extract = tl->external;
            t->name.begin = alloc->cursor;
            push_string_data(*tl->name, alloc);
            t->name.end = alloc->cursor;
            t->build = &dummy;
            t->template = alloc->cursor;
            t->deps = alloc->cursor;
            push_string(*extract->tar, alloc);
            push_string(*extract->marker, alloc);
            for (string* d = tl->deps.begin; (void*)d < tl->deps.end;) {
                string* s = sa_alloc(alloc, sizeof(*s));
                s->begin = alloc->cursor;
                push_string_data(*d, alloc);
                s->end = alloc->cursor;
                d = (void*)d->end;
            }
            t->end = alloc->cursor;
        } break;
        }
        tl = tl->end;
    }
    
    sa_move_tail(alloc, targets_local.end, targets_local.begin);
    targets_offset((void*)targets_local.begin, -bytesize(targets_local.begin, targets_local.end), alloc);
    targets targetss; targetss.begin = (void*)targets_local.begin;
    targetss.end = alloc->cursor;
    targets_local.begin = 0; targets_local.end = 0;

    target* target_snake = 0;
    target* target_dummy = 0;
    target* target_tests = 0;

    for (target* t = targetss.begin; (void*)t < targetss.end;) {
        const string target_snake_name = STR("build_minimake/snake");
        const string target_dummy_name = STR("build_minimake/dummy");
        const string target_tests_name = STR("build_minimake/run_tests");
        if (sa_equals(alloc, t->name.begin, t->name.end, target_snake_name.begin, target_snake_name.end)) {
            target_snake = t;
        }
        if (sa_equals(alloc, t->name.begin, t->name.end, target_dummy_name.begin, target_dummy_name.end)) {
            target_dummy = t;
        }
        if (sa_equals(alloc, t->name.begin, t->name.end, target_tests_name.begin, target_tests_name.end)) {
            target_tests = t;
        }
        if (target_snake && target_dummy && target_tests) {break;}
        t = t->end;
    }

    i32 return_code = 0;
    if (!target_build(targetss.begin, targetss.end, target_snake, cache_dir, alloc)) {
        return_code = 1;
    }
    if (!target_build(targetss.begin, targetss.end, target_dummy, cache_dir, alloc)) {
        return_code = 1;
    }
    if (!target_build(targetss.begin, targetss.end, target_tests, cache_dir, alloc)) {
        return_code = 1;
    }
    
    sa_free(alloc, memory);

    sa_deinit(alloc);
    mem_unmap(memory, memory_size);
    return return_code;
}
