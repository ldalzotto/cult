#include "target_build.h"

#include "target_execution_list.h"
#include "print.h"
#include "target_timestamp.h"

u8 target_build_phony(target* t, u8 dry, exec_command_session* session, string cache_dir, stack_alloc* alloc) {
    unused(session);
    if (!dry) {
        target_update_timestamp(t, cache_dir, alloc);
    }
    return 1;
}

u8 target_build_object_dependencies(target* t, u8 dry, exec_command_session* session, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;

    const string template = {t->template, t->deps};
    string c_file = {alloc->cursor, alloc->cursor};
    for (string* d = t->deps; (void*)d < t->end;) {
        const string c_extension = STR(".c");
        if (sa_contains(alloc, d->begin, d->end, c_extension.begin, c_extension.end)) {
            c_file = *d;
            break;
        }

        d = (void*)d->end;
    }
    string dep_file = t->name;
    directory_create_for_file(alloc, dep_file.begin, dep_file.end, DIR_MODE_PUBLIC);

    string command;
    command.begin = print_format_to_buffer(alloc, template, c_file, dep_file);
    command.end = alloc->cursor;

    print_format(file_stdout(), STRING("%s\n"), command);

    exec_command_result exec;
    if (!dry) {
        exec = command_session_exec_command(session, command, alloc);
    } else {
        exec.output = alloc->cursor;
        exec.success = 1;
    }
    string log;
    log.begin = exec.output;
    log.end = alloc->cursor;
    if (log.begin != log.end) {
        print_format(file_stdout(), STRING("%s\n"), log);
    }
    exec.output = 0;

    sa_free(alloc, begin);

    if (exec.success && !dry) {
        target_update_timestamp(t, cache_dir, alloc);
    }

    return exec.success;
}

u8 target_build_object(target* t, u8 dry, exec_command_session* session, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    
    const string template = {t->template, t->deps};
    string c_file = {alloc->cursor, alloc->cursor};
    for (string* d = t->deps; (void*)d < t->end;) {
        const string c_extension = STR(".c");
        if (sa_contains(alloc, d->begin, d->end, c_extension.begin, c_extension.end)) {
            c_file = *d;
            break;
        }

        d = (void*)d->end;
    }

    directory_create_for_file(alloc, t->name.begin, t->name.end, DIR_MODE_PUBLIC);

    string command;
    command.begin = print_format_to_buffer(alloc, template, c_file, t->name);
    command.end = alloc->cursor;

    print_format(file_stdout(), STRING("%s\n"), command);

    exec_command_result exec;
    if (!dry) {
        exec = command_session_exec_command(session, command, alloc);
    } else {
        exec.output = alloc->cursor;
        exec.success = 1;
    }
    string log;
    log.begin = exec.output;
    log.end = alloc->cursor;
    if (log.begin != log.end) {
        print_format(file_stdout(), STRING("%s\n"), log);
    }
    exec.output = 0;
    
    sa_free(alloc, begin);

    if (exec.success && !dry) {
        target_update_timestamp(t, cache_dir, alloc);
    }

    return exec.success;
}

u8 target_build_executable(target* t, u8 dry, exec_command_session* session, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    
    const string template = {t->template, t->deps};
    
    string deps_as_command;
    deps_as_command.begin = alloc->cursor;
    for (string* d = t->deps; (void*)d < t->end;) {
        const string extension = STRING(".o");
        if (!sa_contains(alloc, d->begin, d->end, extension.begin, extension.end)) {
            goto next;
        }
        void* cursor = sa_alloc(alloc, bytesize(d->begin, d->end));
        sa_copy(alloc, d->begin, cursor, bytesize(d->begin, d->end));
        cursor = sa_alloc(alloc, 1);
        *(u8*)cursor = ' ';
        next:
        d = (void*)d->end;
    }
    deps_as_command.end = alloc->cursor;

    string command;
    command.begin = print_format_to_buffer(alloc, template, deps_as_command, t->name);
    command.end = alloc->cursor;

    print_format(file_stdout(), STRING("%s\n"), command);

    exec_command_result exec;
    if (!dry) {
        exec = command_session_exec_command(session, command, alloc);
    } else {
        exec.output = alloc->cursor;
        exec.success = 1;
    }
    string log;
    log.begin = exec.output;
    log.end = alloc->cursor;
    if (log.begin != log.end) {
        print_format(file_stdout(), STRING("%s\n"), log);
    }
    exec.output = 0;
    
    sa_free(alloc, begin);

    if (exec.success && !dry) {
        target_update_timestamp(t, cache_dir, alloc);
    }
    return exec.success;
}

u8 target_build_extract(target* t, u8 dry, exec_command_session* session, string cache_dir, stack_alloc* alloc) {
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
        const string tar_command = STRING("tar -xzvf ");
        const string output_command = STRING(" -C ");

        void* cursor;
        cursor = sa_alloc(alloc, bytesize(tar_command.begin, tar_command.end));
        sa_copy(alloc, tar_command.begin, cursor, bytesize(tar_command.begin, tar_command.end));

        cursor = sa_alloc(alloc, bytesize(tar_gz_input_path.begin, tar_gz_input_path.end));
        sa_copy(alloc, tar_gz_input_path.begin, cursor, bytesize(tar_gz_input_path.begin, tar_gz_input_path.end));

        cursor = sa_alloc(alloc, bytesize(output_command.begin, output_command.end));
        sa_copy(alloc, output_command.begin, cursor, bytesize(output_command.begin, output_command.end));

        cursor = sa_alloc(alloc, bytesize(output_directory.begin, output_directory.end));
        sa_copy(alloc, output_directory.begin, cursor, bytesize(output_directory.begin, output_directory.end));

        command.end = alloc->cursor;

        // print_format(file_stdout(), STRING("%s\n"), command);
        exec_command_result exec;
        if (!dry) {
            exec = command_session_exec_command(session, command, alloc);
        } else {
            exec.output = alloc->cursor;
            exec.success = 1;
        }
        string output = {exec.output, alloc->cursor};
        print_format(file_stdout(), STRING("%s"), output);
        
        success = exec.success;
        sa_free(alloc, begin);
    }

    // Create the marker
    if (success) {
        file_t file = file_open(alloc, marker_path.begin, marker_path.end, FILE_MODE_READ_WRITE);
        file_close(file);
    }

    if (success && !dry) {
        target_update_timestamp(t, cache_dir, alloc);
    }

    return success;
}

static u8 target_should_build(target* t, string cache_dir, stack_alloc* alloc) {
    if (t->deps == t->end) {
        return 1;
    }

    uptr input_ts = timestamp_read(t->name, cache_dir, alloc);
    for (const string* d = t->deps; (void*)d < t->end;) {
        uptr dep_ts = 0;
        file_t file = file_open(alloc, d->begin, d->end, FILE_MODE_READ);
        if (file != file_invalid()) {
            dep_ts = file_modification_time(file);
            file_close(file);
        } else {
            dep_ts = timestamp_read(*d, cache_dir, alloc);
        }
    
        if (dep_ts > input_ts) {
            return 1;
        }

        d = d->end;
    }
    return 0; 
}


u8 target_build(target* targets_begin, target* targets_end, target* target_to_build, u8 dry, exec_command_session* session, string cache_dir, stack_alloc* alloc) {
    void* begin = alloc->cursor;
    u8 success = 1;
    // Compute the target execution list
    struct { target** begin; target** end; } execution_list;
    execution_list.begin = target_execution_list(targets_begin, targets_end, target_to_build, alloc);
    execution_list.end = alloc->cursor;

    for (target** target_pp = execution_list.end - 1; target_pp >= execution_list.begin; --target_pp) {
        target* t = *target_pp;

        if (target_should_build(t, cache_dir, alloc)) {
             print_format(file_stdout(), STRING("%s\n"), t->name);
            if (!t->build(t, dry, session, cache_dir, alloc)) {
                success = 0;
                break;
            }
        }
    }

    sa_free(alloc, begin);
    return success;
}
