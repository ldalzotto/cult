#ifndef MINIMAKE_SCRIPT_H
#define MINIMAKE_SCRIPT_H

#include "target.h"

/* Re-used simple container typedefs */
typedef struct { string* begin; void* end; } strings;
typedef struct { target* begin; void* end; } targets;

/* Utility functions used by minimake's make_targets implementation */

target* targets_offset(target* target, uptr offset, stack_alloc* alloc);

string begin_string(stack_alloc* alloc);
void end_string(string* s, stack_alloc* alloc);

/* Push a single string into the given stack allocator (stores a string struct pointing to copied bytes) */
void push_string(const string s, stack_alloc* alloc);

strings begin_strings(stack_alloc* alloc);
void end_strings(strings* s, stack_alloc* alloc);

/* Push multiple strings (a 'strings' sequence) by copying each element into alloc */
void push_strings(const strings s, stack_alloc* alloc);

/* Push raw string bytes into alloc (no string struct header) */
void push_string_data(const string s, stack_alloc* alloc);

typedef struct {
    strings c;
    strings o;
    strings d;
} c_object_files;

c_object_files make_c_object_files(strings sources, string build_dir, stack_alloc* alloc);

/* High level target creators used by make_targets */
targets create_c_object_targets(const string cc, const strings flags,
        c_object_files files,
        strings additional_deps,
        stack_alloc* alloc);

target* create_executable_target(const string cc, const strings flags, string build_dir, string executable_file,
    strings deps,
     stack_alloc* alloc);

target* create_extract_target(const string targz_file, const string marker_file, const string timestamp_file, stack_alloc* alloc);

u8 target_build_name(targets targets, string target, string build_dir, string cache_dir,  exec_command_session* session, stack_alloc* alloc);

#endif // MINIMAKE_SCRIPT_H
