#ifndef CONVERT_H
#define CONVERT_H

#include "primitive.h"
#include "stack_alloc.h"

// Function declarations for converting primitive types to strings
char* convert_i8_to_string(i8 value, stack_alloc* alloc);
char* convert_u8_to_string(u8 value, stack_alloc* alloc);
char* convert_i16_to_string(i16 value, stack_alloc* alloc);
char* convert_u16_to_string(u16 value, stack_alloc* alloc);
char* convert_i32_to_string(i32 value, stack_alloc* alloc);
char* convert_u32_to_string(u32 value, stack_alloc* alloc);
char* convert_i64_to_string(i64 value, stack_alloc* alloc);
char* convert_u64_to_string(u64 value, stack_alloc* alloc);
char* convert_iptr_to_string(iptr value, stack_alloc* alloc);
char* convert_uptr_to_string(uptr value, stack_alloc* alloc);
char* convert_pointer_to_string(void* ptr, stack_alloc* alloc);

#endif /* CONVERT_H */
