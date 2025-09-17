#ifndef CONVERT_H
#define CONVERT_H

#include "primitive.h"

// TODO: should take a stack alloc as well
// Function declarations for converting primitive types to strings
void convert_i8_to_string(i8 value, char* buffer, uptr* length);
void convert_u8_to_string(u8 value, char* buffer, uptr* length);
void convert_i16_to_string(i16 value, char* buffer, uptr* length);
void convert_u16_to_string(u16 value, char* buffer, uptr* length);
void convert_i32_to_string(i32 value, char* buffer, uptr* length);
void convert_u32_to_string(u32 value, char* buffer, uptr* length);
void convert_i64_to_string(i64 value, char* buffer, uptr* length);
void convert_u64_to_string(u64 value, char* buffer, uptr* length);
void convert_iptr_to_string(iptr value, char* buffer, uptr* length);
void convert_uptr_to_string(uptr value, char* buffer, uptr* length);
void convert_pointer_to_string(void* ptr, char* buffer, uptr* length);

#endif /* CONVERT_H */
