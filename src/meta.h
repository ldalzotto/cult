#ifndef META_H
#define META_H

#include "./primitive.h"
#include "./litteral.h"

// Forward declaration
typedef struct meta meta;
typedef struct field_descriptor field_descriptor;

// Enum for primitive types
typedef enum {
    PT_NONE,  // For structs/complex types
    PT_I8, PT_U8, PT_I16, PT_U16,
    PT_I32, PT_U32, PT_I64, PT_U64,
    PT_IPTR, PT_UPTR
} primitive_type;

// Meta structure for describing how to print any type
struct meta {
    string_span type_name;         // e.g., "i32", "my_struct"
    u32 is_array;                  // 1 if this is an array type
    uptr type_size;                // size of the type in bytes
    primitive_type pt;             // primitive type enum
    struct {
        const field_descriptor* begin; // pointer to first field descriptor
        const field_descriptor* end; // pointer to after last field descriptor
    } fields;
};

// Field descriptor structure
struct field_descriptor {
    string_span field_name;
    uptr offset;              // byte offset in struct
    const meta* field_meta;   // recursive meta for nested types
};


static const meta i8_meta = {
    .type_name = STATIC_STRING("i8"),
    .is_array = 0,
    .type_size = sizeof(i8),
    .pt = PT_I8,
    .fields = {0,0},
};

static const meta u8_meta = {
    .type_name = STATIC_STRING("u8"),
    .is_array = 0,
    .type_size = sizeof(u8),
    .pt = PT_U8,
    .fields = {0,0},
};

static const meta i16_meta = {
    .type_name = STATIC_STRING("i16"),
    .is_array = 0,
    .type_size = sizeof(i16),
    .pt = PT_I16,
    .fields = {0,0},
};

static const meta u16_meta = {
    .type_name = STATIC_STRING("u16"),
    .is_array = 0,
    .type_size = sizeof(u16),
    .pt = PT_U16,
    .fields = {0,0},
};

static const meta i32_meta = {
    .type_name = STATIC_STRING("i32"),
    .is_array = 0,
    .type_size = sizeof(i32),
    .pt = PT_I32,
    .fields = {0,0},
};

static const meta u32_meta = {
    .type_name = STATIC_STRING("u32"),
    .is_array = 0,
    .type_size = sizeof(u32),
    .pt = PT_U32,
    .fields = {0,0},
};

static const meta i64_meta = {
    .type_name = STATIC_STRING("i64"),
    .is_array = 0,
    .type_size = sizeof(i64),
    .pt = PT_I64,
    .fields = {0,0},
};

static const meta u64_meta = {
    .type_name = STATIC_STRING("u64"),
    .is_array = 0,
    .type_size = sizeof(u64),
    .pt = PT_U64,
    .fields = {0,0},
};

static const meta iptr_meta = {
    .type_name = STATIC_STRING("iptr"),
    .is_array = 0,
    .type_size = sizeof(iptr),
    .pt = PT_IPTR,
    .fields = {0,0},
};

static const meta uptr_meta = {
    .type_name = STATIC_STRING("uptr"),
    .is_array = 0,
    .type_size = sizeof(uptr),
    .pt = PT_UPTR,
    .fields = {0,0},
};

#endif /* META_H */
