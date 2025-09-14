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

// Print a plain string to file
void print_string(file_t file, const char* str) {
    uptr len = 0;
    while (str[len] != '\0') ++len;
    file_write(file, str, len);
}

// Format segment types
typedef enum {
    FORMAT_SEGMENT_LITERAL,    // Literal text
    FORMAT_SEGMENT_SPECIFIER,  // Format specifier (%d, %s, etc.)
    FORMAT_SEGMENT_END         // End of format string
} format_segment_type;

// Format iterator structure
typedef struct {
    const char* format;        // Original format string
    const char* current;       // Current position in format string
    const char* segment_start; // Start of current segment
    const char* segment_end;   // End of current segment
    format_segment_type type;  // Type of current segment
    char specifier;            // Format specifier character (for FORMAT_SEGMENT_SPECIFIER)
} format_iterator;

// Initialize format iterator
static void format_iterator_init(format_iterator* iter, const char* format) {
    iter->format = format;
    iter->current = format;
    iter->segment_start = format;
    iter->segment_end = format;
    iter->type = FORMAT_SEGMENT_LITERAL;
    iter->specifier = '\0';
}

// Advance iterator to next segment
static void format_iterator_next(format_iterator* iter) {
    if (*iter->current == '\0') {
        iter->type = FORMAT_SEGMENT_END;
        return;
    }

    if (*iter->current == '%') {
        // Handle format specifier
        iter->segment_start = iter->current;
        iter->current++; // Skip '%'

        if (*iter->current == '\0') {
            // Trailing '%' at end of string
            iter->segment_end = iter->current;
            iter->type = FORMAT_SEGMENT_LITERAL;
            return;
        }

        iter->specifier = *iter->current;
        iter->segment_end = iter->current + 1;
        iter->current++; // Skip specifier
        iter->type = FORMAT_SEGMENT_SPECIFIER;
    } else {
        // Handle literal text
        iter->segment_start = iter->current;
        iter->type = FORMAT_SEGMENT_LITERAL;

        // Find next '%' or end of string
        while (*iter->current != '\0' && *iter->current != '%') {
            iter->current++;
        }
        iter->segment_end = iter->current;
    }
}

// Get segment text and length
static void format_iterator_get_segment(const format_iterator* iter, const char** text, uptr* length) {
    *text = iter->segment_start;
    *length = iter->segment_end - iter->segment_start;
}

// Process a format specifier and get the result
static void process_format_specifier(char specifier, va_list args, char* buffer, uptr* length) {
    switch (specifier) {
        case 'd': {
            // Handle signed integer
            i32 value = va_arg(args, i32);
            convert_i32_to_string(value, buffer, length);
            break;
        }
        case 'u': {
            // Handle unsigned integer
            u32 value = va_arg(args, u32);
            convert_u32_to_string(value, buffer, length);
            break;
        }
        case 's': {
            // Handle string
            const char* str = va_arg(args, const char*);
            if (str) {
                *length = 0;
                while (str[*length] != '\0') ++(*length);
                // Copy string to buffer
                for (uptr i = 0; i < *length; i++) {
                    buffer[i] = str[i];
                }
            } else {
                // Copy "(null)" to buffer
                const char* null_str = "(null)";
                *length = 6;
                for (uptr i = 0; i < *length; i++) {
                    buffer[i] = null_str[i];
                }
            }
            break;
        }
        case 'c': {
            // Handle character
            char c = (char)va_arg(args, int);  // char is promoted to int
            buffer[0] = c;
            *length = 1;
            break;
        }
        case 'p': {
            // Handle pointer
            void* ptr = va_arg(args, void*);
            convert_pointer_to_string(ptr, buffer, length);
            break;
        }
        default:
            // Handle unknown format specifier
            buffer[0] = '%';
            buffer[1] = specifier;
            *length = 2;
            break;
    }
}

// Print a formatted string with arguments to file
void print_format(file_t file, const char* format, ...) {
    debug_assert(format != 0);

    va_list args;
    va_start(args, format);

    format_iterator iter;
    format_iterator_init(&iter, format);

    while (iter.type != FORMAT_SEGMENT_END) {
        format_iterator_next(&iter);

        if (iter.type == FORMAT_SEGMENT_LITERAL) {
            const char* text;
            uptr length;
            format_iterator_get_segment(&iter, &text, &length);
            file_write(file, text, length);
        } else if (iter.type == FORMAT_SEGMENT_SPECIFIER) {
            char buffer[256];
            uptr length;
            process_format_specifier(iter.specifier, args, buffer, &length);
            file_write(file, buffer, length);
        }
    }

    va_end(args);
}

// Print a formatted string with arguments to a buffer using stack allocator
void* print_format_to_buffer(stack_alloc* alloc, const char* format, ...) {
    debug_assert(alloc != 0);
    debug_assert(format != 0);

    void* start = alloc->cursor;

    va_list args;
    va_start(args, format);

    format_iterator iter;
    format_iterator_init(&iter, format);

    while (iter.type != FORMAT_SEGMENT_END) {
        format_iterator_next(&iter);

        if (iter.type == FORMAT_SEGMENT_LITERAL) {
            const char* text;
            uptr length;
            format_iterator_get_segment(&iter, &text, &length);
            void* dest = sa_alloc(alloc, length);
            memcpy(dest, text, length);
        } else if (iter.type == FORMAT_SEGMENT_SPECIFIER) {
            char buffer[256];
            uptr length;
            process_format_specifier(iter.specifier, args, buffer, &length);
            void* dest = sa_alloc(alloc, length);
            memcpy(dest, buffer, length);
        }
    }

    va_end(args);

    return start;
}
