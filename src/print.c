#include "print.h"
#include <stddef.h>  // for NULL
#include <stdio.h>   // for sprintf
#include <stdarg.h>  // for variadic functions

// Helper function to convert i8 to string
static void convert_i8_to_string(i8 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Handle negative numbers
    int is_negative = 0;
    i8 temp = value;
    if (temp < 0) {
        is_negative = 1;
        temp = -temp;
    }
    
    // Convert digits
    char temp_buf[32];
    int temp_idx = 0;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert u8 to string
static void convert_u8_to_string(u8 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Convert digits
    char temp_buf[32];
    int temp_idx = 0;
    u8 temp = value;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert i16 to string
static void convert_i16_to_string(i16 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Handle negative numbers
    int is_negative = 0;
    i16 temp = value;
    if (temp < 0) {
        is_negative = 1;
        temp = -temp;
    }
    
    // Convert digits
    char temp_buf[32];
    int temp_idx = 0;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert u16 to string
static void convert_u16_to_string(u16 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Convert digits
    char temp_buf[32];
    int temp_idx = 0;
    u16 temp = value;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert i32 to string
static void convert_i32_to_string(i32 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Handle negative numbers
    int is_negative = 0;
    i32 temp = value;
    if (temp < 0) {
        is_negative = 1;
        temp = -temp;
    }
    
    // Convert digits
    char temp_buf[32];
    int temp_idx = 0;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert u32 to string
static void convert_u32_to_string(u32 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Convert digits
    char temp_buf[32];
    int temp_idx = 0;
    u32 temp = value;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert i64 to string
static void convert_i64_to_string(i64 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Handle negative numbers
    int is_negative = 0;
    i64 temp = value;
    if (temp < 0) {
        is_negative = 1;
        temp = -temp;
    }
    
    // Convert digits
    char temp_buf[64];
    int temp_idx = 0;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert u64 to string
static void convert_u64_to_string(u64 value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Convert digits
    char temp_buf[64];
    int temp_idx = 0;
    u64 temp = value;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert iptr to string
static void convert_iptr_to_string(iptr value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Handle negative numbers
    int is_negative = 0;
    iptr temp = value;
    if (temp < 0) {
        is_negative = 1;
        temp = -temp;
    }
    
    // Convert digits
    char temp_buf[64];
    int temp_idx = 0;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        buffer[idx++] = '-';
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Helper function to convert uptr to string
static void convert_uptr_to_string(uptr value, char* buffer, uptr* length) {
    int idx = 0;
    
    // Handle special case of 0
    if (value == 0) {
        buffer[idx++] = '0';
        *length = idx;
        return;
    }
    
    // Convert digits
    char temp_buf[64];
    int temp_idx = 0;
    uptr temp = value;
    while (temp > 0) {
        temp_buf[temp_idx++] = '0' + (temp % 10);
        temp /= 10;
    }
    
    // Reverse the digits
    for (int i = temp_idx - 1; i >= 0; i--) {
        buffer[idx++] = temp_buf[i];
    }
    
    *length = idx;
}

// Generic print function
void print_generic(const print_meta* meta, void* data, file_t file, u32 indent_level) {
    if (meta->pt != PT_NONE) {
        // Primitive
        char buf[256];
        uptr len;
        switch (meta->pt) {
            case PT_I8: convert_i8_to_string(*(i8*)data, buf, &len); break;
            case PT_U8: convert_u8_to_string(*(u8*)data, buf, &len); break;
            case PT_I16: convert_i16_to_string(*(i16*)data, buf, &len); break;
            case PT_U16: convert_u16_to_string(*(u16*)data, buf, &len); break;
            case PT_I32: convert_i32_to_string(*(i32*)data, buf, &len); break;
            case PT_U32: convert_u32_to_string(*(u32*)data, buf, &len); break;
            case PT_I64: convert_i64_to_string(*(i64*)data, buf, &len); break;
            case PT_U64: convert_u64_to_string(*(u64*)data, buf, &len); break;
            case PT_IPTR: convert_iptr_to_string(*(iptr*)data, buf, &len); break;
            case PT_UPTR: convert_uptr_to_string(*(uptr*)data, buf, &len); break;
            default: 
                len = 18;
                for (uptr i = 0; i < len; i++) buf[i] = "<unknown primitive>"[i];
                break;
        }
        file_write(file, buf, len);
    } else {
        // Struct
        file_write(file, meta->type_name.begin, bytesize(meta->type_name.begin, meta->type_name.end));
        file_write(file, " {\n", 3);
        field_descriptor* field = (field_descriptor*)meta->fields_begin;
        field_descriptor* field_end = (field_descriptor*)meta->fields_end;
        for (; field < field_end; ++field) {
            // Indentation
            for (u32 i = 0; i < indent_level + 1; ++i) {
                file_write(file, "  ", 2);
            }
            file_write(file, field->field_name.begin, bytesize(field->field_name.begin, field->field_name.end));
            file_write(file, ": ", 2);
            void* field_data = byteoffset(data, field->offset);
            print_generic(field->field_meta, field_data, file, indent_level + 1);
            file_write(file, "\n", 1);
        }
        // Closing brace with indentation
        for (u32 i = 0; i < indent_level; ++i) {
            file_write(file, "  ", 2);
        }
        file_write(file, "}", 1);
    }
}

// Specialized function for printing arrays
void print_array_generic(const print_meta* element_meta, void* begin, void* end, file_t file, u32 indent_level) {
    file_write(file, "[", 1);
    if (begin < end) {
        void* current = begin;
        print_generic(element_meta, current, file, indent_level);
        current = byteoffset(current, element_meta->type_size);
        while (current < end) {
            file_write(file, ", ", 2);
            print_generic(element_meta, current, file, indent_level);
            current = byteoffset(current, element_meta->type_size);
        }
    }
    file_write(file, "]", 1);
}

// Print a plain string to file
void print_string(file_t file, const char* str) {
    if (str) {
        uptr len = 0;
        while (str[len] != '\0') ++len;
        file_write(file, str, len);
    }
}

// Print a formatted string with arguments to file
void print_format(file_t file, const char* format, ...) {
    if (!format) return;
    
    va_list args;
    va_start(args, format);
    
    const char* current = format;
    const char* start = format;
    
    while (*current != '\0') {
        if (*current == '%') {
            // Write the accumulated string before the format specifier
            if (current > start) {
                file_write(file, start, current - start);
            }
            
            // Move past the '%'
            current++;
            
            // Handle the format specifier
            switch (*current) {
                case 'd': {
                    // Handle signed integer
                    i32 value = va_arg(args, i32);
                    
                    // Convert integer to string using our helper function
                    char int_buf[32];
                    uptr len;
                    convert_i32_to_string(value, int_buf, &len);
                    file_write(file, int_buf, len);
                    break;
                }
                case 'u': {
                    // Handle unsigned integer
                    u32 value = va_arg(args, u32);
                    
                    // Convert unsigned integer to string using our helper function
                    char uint_buf[32];
                    uptr len;
                    convert_u32_to_string(value, uint_buf, &len);
                    file_write(file, uint_buf, len);
                    break;
                }
                case 's': {
                    // Handle string
                    const char* str = va_arg(args, const char*);
                    if (str) {
                        uptr len = 0;
                        while (str[len] != '\0') ++len;
                        file_write(file, str, len);
                    } else {
                        file_write(file, "(null)", 6);
                    }
                    break;
                }
                case 'c': {
                    // Handle character
                    char c = (char)va_arg(args, int);  // char is promoted to int
                    file_write(file, &c, 1);
                    break;
                }
                default:
                    // Handle unknown format specifier by just printing it
                    file_write(file, "%", 1);
                    if (*current) {
                        file_write(file, current, 1);
                    }
                    break;
            }
            
            // Move past the format specifier
            if (*current) {
                current++;
            }
            start = current;
        } else {
            // Regular character, move to next
            current++;
        }
    }
    
    // Write any remaining string after the last format specifier
    if (current > start) {
        file_write(file, start, current - start);
    }
    
    va_end(args);
}
