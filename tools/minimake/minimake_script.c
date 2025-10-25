#include "minimake_script.h"

#include "target_build.h"
#include "target_c_dependencies.h"
#include "exec_command.h"
#include "file.h"

/* Internal helpers for pointer adjustment of targets stored in the stack allocator */
static void target_offset(target* t, uptr offset) {
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

/* Helper utilities to simplify target creation (moved from minimake.c) */
static target* create_target(stack_alloc* alloc, const string name, u8 (*build)(target*, u8, exec_command_session*, string, stack_alloc*)) {
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

/* Strings utilities */

string begin_string(stack_alloc* alloc) {return (string){.begin = alloc->cursor};}
void end_string(string* s, stack_alloc* alloc) {s->end = alloc->cursor;}

void push_string(const string s, stack_alloc* alloc) {
    string* sp = sa_alloc(alloc, sizeof(string));
    sp->begin = sa_alloc(alloc, bytesize(s.begin, s.end));
    sa_copy(alloc, s.begin, (void*)sp->begin, bytesize(s.begin, s.end));
    sp->end = alloc->cursor;
}


strings begin_strings(stack_alloc* alloc) {return (strings){.begin = alloc->cursor};}
void end_strings(strings* s, stack_alloc* alloc) {s->end = alloc->cursor;}

void push_strings(const strings s, stack_alloc* alloc) {
    for (string* str = s.begin; (void*)str < s.end;) {
        push_string(*str, alloc);
        str = (void*)str->end;
    }
}

void push_string_data(const string s, stack_alloc* alloc) {
    void* cursor = sa_alloc(alloc, bytesize(s.begin, s.end));
    sa_copy(alloc, s.begin, cursor, bytesize(s.begin, s.end));
}

/* Prepend string data: write prepend_value then the string_to_prepend into alloc */
static void preprend_string_data(const string string_to_prepend, const string prepend_value, stack_alloc* alloc) {
    push_string_data(prepend_value, alloc);
    push_string_data(string_to_prepend, alloc);
}

/* Template builders */
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
    push_string_data(STRING("-MM %s -MF %s"), alloc);
}

/* Helpers to derive object filenames from sources */
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

static strings get_c_object_dep_names(strings sources, string build_dir, stack_alloc* alloc) {
    strings names; names.begin = alloc->cursor;
    for (string* source = sources.begin; (void*)source<sources.end;) {
        string* s = sa_alloc(alloc, sizeof(*s));
        s->begin = alloc->cursor;
        preprend_string_data(*source, build_dir, alloc);
        s->end = alloc->cursor;
        *((u8*)s->end - 1) = 'd';
        source = (void*)source->end;
    }

    names.end = alloc->cursor;
    return names;
}

c_object_files make_c_object_files(strings sources, string build_dir, stack_alloc* alloc) {
    c_object_files result;
    result.c = sources;
    result.o = get_c_object_names(sources, build_dir, alloc);
    result.d = get_c_object_dep_names(sources, build_dir, alloc);
    return result;
}

string make_c_executable_file(string name, string build_dir, stack_alloc* alloc) {
    string s;
    s.begin = alloc->cursor;
    push_string_data(build_dir, alloc);
    push_string_data(name, alloc);
    s.end = alloc->cursor;
    return s;
}

target* create_phony_target(const string name, const strings deps, string build_dir, stack_alloc* alloc) {
    void* var_begin = alloc->cursor;
    string name_pre; name_pre.begin = alloc->cursor;
    push_string_data(build_dir, alloc);
    push_string_data(name, alloc);
    name_pre.end = alloc->cursor;
    void* var_end = alloc->cursor;

    target* t = create_target(alloc, name_pre, target_build_phony);
    t->template = alloc->cursor;
    t->deps = alloc->cursor;
    push_strings(deps, alloc);
    finish_target(t, alloc);

    sa_move_tail(alloc, var_end, var_begin);
    targets_offset(var_begin, -bytesize(var_begin, var_end), alloc);
    return var_begin;
}

/* Create object build targets and their dependency-extraction targets */
targets create_c_object_targets(const string cc, const strings flags,
        c_object_files files,
        strings additional_deps,
        stack_alloc* alloc)
{
    void* begin = alloc->cursor;

    string* object = files.o.begin;
    string* object_dependency = files.d.begin;
    for (string* source = files.c.begin; (void*)source < files.c.end;) {
        void* var_begin = alloc->cursor;
        
        string build_object_template; build_object_template.begin = alloc->cursor;
        make_build_object_template(alloc, cc, flags);
        build_object_template.end = alloc->cursor;

        string extract_dependency_template; extract_dependency_template.begin = alloc->cursor;
        make_extract_dependency_template(alloc, cc, flags);
        extract_dependency_template.end = alloc->cursor;
      
        void* var_end = alloc->cursor;

        target* target = create_target(alloc, *object, target_build_object);
        target->template = sa_alloc(alloc, bytesize(build_object_template.begin, build_object_template.end));
        sa_copy(alloc, build_object_template.begin, target->template, bytesize(build_object_template.begin, build_object_template.end));
        target->deps = alloc->cursor;
        for (string* d = additional_deps.begin; (void*)d < additional_deps.end;) {
            push_string(*d, alloc);
            d = (void*)d->end;
        }
        push_string(*object_dependency, alloc);
        push_string(*source, alloc);
        finish_target(target, alloc);

        struct target* dependency_target = create_target(alloc, *object_dependency, target_build_object_dependencies);
        dependency_target->template = sa_alloc(alloc, bytesize(extract_dependency_template.begin, extract_dependency_template.end));
        sa_copy(alloc, extract_dependency_template.begin, dependency_target->template, bytesize(extract_dependency_template.begin, extract_dependency_template.end));
        dependency_target->deps = alloc->cursor;
        push_string(*source, alloc);

        // if dependency file available, parse it
        file_t dep_file = file_open(alloc, object_dependency->begin, object_dependency->end, FILE_MODE_READ);
        if (dep_file != file_invalid()) {
            void* var_begin = alloc->cursor;
            string dep_file_content;
            file_read_all(dep_file, (void**)&dep_file_content.begin, alloc);
            file_close(dep_file);
            dep_file_content.end = alloc->cursor;
            void* var_end = alloc->cursor;
            string* dependencies = extract_c_dependencies(dep_file_content, alloc);
            
            sa_move_tail(alloc, var_end, var_begin);
            const uptr offset = -bytesize(var_begin, var_end);
            dependencies = var_begin;
            for (string* d = dependencies; (void*)d<alloc->cursor;) {
                d->begin = byteoffset(d->begin, offset);
                d->end = byteoffset(d->end, offset);
                d = (void*)d->end;
            }
        }
        finish_target(dependency_target, alloc);

        const uptr offset = bytesize(var_begin, var_end);
        sa_move_tail(alloc, var_end, var_begin);
        targets_offset(var_begin, -offset, alloc);

        source = (void*)source->end;
        object = (void*)object->end;
        object_dependency = (void*)object_dependency->end;
    }
    return (targets){.begin = begin, alloc->cursor};
}

/* Create executable linking target */
target* create_executable_target(const string cc, const strings flags, string executable_file, strings deps, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    void* var_begin = alloc->cursor;

    string executable_link_template; executable_link_template.begin = alloc->cursor;
    make_executable_link_template(alloc, cc, flags);
    executable_link_template.end = alloc->cursor;

    void* var_end = alloc->cursor;

    target* target = create_target(alloc, executable_file, target_build_executable);
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

/* Create extract target (e.g., untar elibs) */
target* create_extract_target(const string targz_file, const string marker_file, const string timestamp_file, stack_alloc* alloc) {
    target* t = create_target(alloc, timestamp_file, target_build_extract);
    t->template = alloc->cursor;
    t->deps = alloc->cursor;
    
    push_string(targz_file, alloc);
    push_string(marker_file, alloc);
    finish_target(t, alloc);

    return t;
}

u8 target_build_name(targets targets, string target, string build_dir, string cache_dir, u8 dry, exec_command_session* session, stack_alloc* alloc) {
    void* begin = alloc->cursor;

    string expected_name; expected_name.begin = alloc->cursor;
    push_string_data(build_dir, alloc);
    push_string_data(target, alloc);
    expected_name.end = alloc->cursor;

    u8 result = 0;
    for (struct target* t = targets.begin; (void*)t<targets.end;) {
        if (sa_equals(alloc, expected_name.begin, expected_name.end, t->name.begin, t->name.end)) {
            result = target_build(targets.begin, targets.end, t, dry, session, cache_dir, alloc);
            break;
        }
        t = t->end;
    }
    
    sa_free(alloc, begin);
    return result;
}
