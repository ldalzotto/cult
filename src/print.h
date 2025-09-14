#ifndef PRINT_H
#define PRINT_H

#include "./primitive.h"
#include "file.h"

// Forward declaration
typedef struct print_meta print_meta;

// Enum for primitive types
typedef enum {
    PT_NONE,  // For structs/complex types
    PT_I8, PT_U8, PT_I16, PT_U16,
    PT_I32, PT_U32, PT_I64, PT_U64,
    PT_IPTR, PT_UPTR
} primitive_type;
typedef struct field_descriptor field_descriptor;

// Meta structure for describing how to print any type
struct print_meta {
    const char* type_name;        // e.g., "i32", "my_struct"
    u32 is_array;                 // 1 if this is an array type
    uptr type_size;               // size of the type in bytes
    primitive_type pt;            // primitive type enum
    const field_descriptor* fields_begin; // pointer to first field descriptor
    const field_descriptor* fields_end; // pointer to after last field descriptor
};

// Field descriptor structure
struct field_descriptor {
    const char* field_name;
    uptr offset;              // byte offset in struct
    const print_meta* field_meta;   // recursive meta for nested types
};

// Generic print function
// For non-arrays: prints the struct/object
// For arrays: assumes data points to begin of contiguous array, but needs end pointer
// For arrays, use print_array_generic instead
void print_generic(const print_meta* meta, void* data, file_t file, u32 indent_level);

// Specialized function for printing arrays using the stack allocator pattern
// begin and end define the contiguous range
void print_array_generic(const print_meta* element_meta, void* begin, void* end, file_t file, u32 indent_level);

// File-based print functions using file.h
static const print_meta i8_meta = {
    .type_name = "i8",
    .is_array = 0,
    .type_size = sizeof(i8),
    .pt = PT_I8,
    .fields_begin = 0,
    .fields_end = 0
};

static const print_meta u8_meta = {
    .type_name = "u8",
    .is_array = 0,
    .type_size = sizeof(u8),
    .pt = PT_U8,
    .fields_begin = 0,
    .fields_end = 0
};

static const print_meta i16_meta = {
    .type_name = "i16",
    .is_array = 0,
    .type_size = sizeof(i16),
    .pt = PT_I16,
    .fields_begin = 0,
    .fields_end = 0
};

static const print_meta u16_meta = {
    .type_name = "u16",
    .is_array = 0,
    .type_size = sizeof(u16),
    .pt = PT_U16,
    .fields_begin = 0,
    .fields_end = 0
};

static const print_meta i32_meta = {
    .type_name = "i32",
    .is_array = 0,
    .type_size = sizeof(i32),
    .pt = PT_I32,
    .fields_begin = 0,
    .fields_end = 0
};

static const print_meta u32_meta = {
    .type_name = "u32",
    .is_array = 0,
    .type_size = sizeof(u32),
    .pt = PT_U32,
    .fields_begin = 0,
    .fields_end = 0
};

static const print_meta i64_meta = {
    .type_name = "i64",
    .is_array = 0,
    .type_size = sizeof(i64),
    .pt = PT_I64,
    .fields_begin = 0,
    .fields_end = 0
};

static const print_meta u64_meta = {
    .type_name = "u64",
    .is_array = 0,
    .type_size = sizeof(u64),
    .pt = PT_U64,
    .fields_begin = 0,
    .fields_end = 0
};

static const print_meta iptr_meta = {
    .type_name = "iptr",
    .is_array = 0,
    .type_size = sizeof(iptr),
    .pt = PT_IPTR,
    .fields_begin = 0,
    .fields_end = 0
};

static const print_meta uptr_meta = {
    .type_name = "uptr",
    .is_array = 0,
    .type_size = sizeof(uptr),
    .pt = PT_UPTR,
    .fields_begin = 0,
    .fields_end = 0
};

#endif /* PRINT_H */
