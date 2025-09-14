#include "print.h"
#include "convert.h"
#include "./assert.h"
#include <stddef.h>  // for NULL
#include <stdio.h>   // for sprintf
#include <stdarg.h>  // for variadic functions
#include <string.h>  // for memcpy

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
    void* current = begin;
    print_generic(element_meta, current, file, indent_level);
    current = byteoffset(current, element_meta->type_size);
    while (current < end) {
        file_write(file, ", ", 2);
        print_generic(element_meta, current, file, indent_level);
        current = byteoffset(current, element_meta->type_size);
    }
    file_write(file, "]", 1);
}

// Print a plain string to file
void print_string(file_t file, const char* str) {
    uptr len = 0;
    while (str[len] != '\0') ++len;
    file_write(file, str, len);
}

// Structure to hold the state of format string iteration
typedef struct {
    const char* current;  // Current position in the format string
    const char* start;    // Start of the current segment
    va_list args;         // Variable arguments list
} format_iterator;

// Structure to hold output context for file output
typedef struct {
    file_t file;
} file_output_context;

// Structure to hold output context for buffer output
typedef struct {
    stack_alloc* alloc;
} buffer_output_context;

// Function pointer type for output handlers
typedef void (*output_handler_func)(void* context, const char* data, uptr len);

// Common format processing function
static void process_format(const char* format, va_list args, void* context, output_handler_func handler) {
    
    format_iterator iter;
    iter.current = format;
    iter.start = format;
    va_copy(iter.args, args);
    
    while (*iter.current != '\0') {
        if (*iter.current == '%') {
            // Output the accumulated string before the format specifier
            if (iter.current > iter.start) {
                handler(context, iter.start, iter.current - iter.start);
            }
            
            // Move past the '%'
            iter.current++;
            
            // Handle the format specifier
            switch (*iter.current) {
                case 'd': {
                    // Handle signed integer
                    i32 value = va_arg(iter.args, i32);
                    
                    // Convert integer to string using our helper function
                    char int_buf[32];
                    uptr len;
                    convert_i32_to_string(value, int_buf, &len);
                    handler(context, int_buf, len);
                    break;
                }
                case 'u': {
                    // Handle unsigned integer
                    u32 value = va_arg(iter.args, u32);
                    
                    // Convert unsigned integer to string using our helper function
                    char uint_buf[32];
                    uptr len;
                    convert_u32_to_string(value, uint_buf, &len);
                    handler(context, uint_buf, len);
                    break;
                }
                case 's': {
                    // Handle string
                    const char* str = va_arg(iter.args, const char*);
                    if (str) {
                        uptr len = 0;
                        while (str[len] != '\0') ++len;
                        handler(context, str, len);
                    } else {
                        handler(context, "(null)", 6);
                    }
                    break;
                }
                case 'c': {
                    // Handle character
                    char c = (char)va_arg(iter.args, int);  // char is promoted to int
                    handler(context, &c, 1);
                    break;
                }
                default:
                    // Handle unknown format specifier by just outputting it
                    handler(context, "%", 1);
                    if (*iter.current) {
                        handler(context, iter.current, 1);
                    }
                    break;
            }
            
            // Move past the format specifier
            if (*iter.current) {
                iter.current++;
            }
            iter.start = iter.current;
        } else {
            // Regular character, move to next
            iter.current++;
        }
    }
    
    // Output any remaining string after the last format specifier
    if (iter.current > iter.start) {
        handler(context, iter.start, iter.current - iter.start);
    }
    
    va_end(iter.args);
}

// Output handler for file output
static void file_output_handler(void* context, const char* data, uptr len) {
    file_output_context* file_ctx = (file_output_context*)context;
    file_write(file_ctx->file, data, len);
}

// Output handler for buffer output
static void buffer_output_handler(void* context, const char* data, uptr len) {
    buffer_output_context* buffer_ctx = (buffer_output_context*)context;
    void* dest = sa_alloc(buffer_ctx->alloc, len);
    memcpy(dest, data, len);
}

// Print a formatted string with arguments to file
void print_format(file_t file, const char* format, ...) {
    debug_assert(format != 0);
    
    va_list args;
    va_start(args, format);
    
    file_output_context context = {file};
    process_format(format, args, &context, file_output_handler);
    
    va_end(args);
}

// Print a formatted string with arguments to a buffer using stack allocator
void* print_format_to_buffer(stack_alloc* alloc, const char* format, ...) {
    debug_assert(alloc != 0);
    debug_assert(format != 0);

    void* start = alloc->cursor;

    va_list args;
    va_start(args, format);
    
    buffer_output_context context = {alloc};
    process_format(format, args, &context, buffer_output_handler);
    
    va_end(args);

    return start;
}
