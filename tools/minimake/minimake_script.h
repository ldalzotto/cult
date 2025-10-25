#ifndef MINIMAKE_SCRIPT_H
#define MINIMAKE_SCRIPT_H

#include "target.h"

/* Re-used simple container typedefs */
typedef struct { string* begin; void* end; } strings;
typedef struct { target* begin; void* end; } targets;

/* Utility functions used by minimake's make_targets implementation */

target* targets_offset(target* target, uptr offset, stack_alloc* alloc);

/* Push a single string into the given stack allocator (stores a string struct pointing to copied bytes) */
void push_string(const string s, stack_alloc* alloc);

/* Push multiple strings (a 'strings' sequence) by copying each element into alloc */
void push_strings(const strings s, stack_alloc* alloc);

/* Push raw string bytes into alloc (no string struct header) */
void push_string_data(const string s, stack_alloc* alloc);

/* Convenience helpers to produce object file names and dependency filenames from source list */
strings get_c_object_names(strings sources, string build_dir, stack_alloc* alloc);
strings get_c_object_dep_names(strings sources, string build_dir, stack_alloc* alloc);

/* High level target creators used by make_targets */
targets create_c_object_targets(const string cc, const strings flags,
        strings sources,
        strings objects,
        strings objects_dependency,
        strings additional_deps,
        stack_alloc* alloc);

target* create_executable_target(const string cc, const strings flags, string build_dir, string executable_file,
    strings deps,
     stack_alloc* alloc);

target* create_extract_target(const string targz_file, const string marker_file, const string timestamp_file, stack_alloc* alloc);

#endif // MINIMAKE_SCRIPT_H
