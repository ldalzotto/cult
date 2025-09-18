#include "format_iterator.h"
#include "convert.h"
#include "./meta_iterator.h"
#include "stack_alloc.h"
#include <stddef.h>
#include <stdarg.h>

// Process a format specifier and get the result
static char* process_format_specifier(char specifier, va_list args, stack_alloc* alloc) {
    switch (specifier) {
        case 'd': {
            // Handle signed integer
            i32 value = va_arg(args, i32);
            return convert_i32_to_string(value, alloc);
        }
        case 'u': {
            // Handle unsigned integer
            u32 value = va_arg(args, u32);
            return convert_u32_to_string(value, alloc);
        }
        case 's': {
            // Handle string
            const char* str = va_arg(args, const char*);
            if (str) {
                uptr length = 0;
                while (str[length] != '\0') ++length;
                char* result = (char*)sa_alloc(alloc, length );
                sa_copy(alloc, str, result, length);
                return result;
            } else {
                // Copy "(null)" to buffer
                const string_span null_str = STR("(null)");
                const uptr null_str_size = bytesize(null_str.begin, null_str.end);
                char* result = sa_alloc(alloc, null_str_size);
                sa_copy(alloc, null_str.begin, result, null_str_size);
                return result;
            }
        }
        case 'c': {
            // Handle character
            char c = (char)va_arg(args, int);  // char is promoted to int
            char* result = sa_alloc(alloc, 1);
            *result = c;
            return result;
        }
        case 'p': {
            // Handle pointer
            void* ptr = va_arg(args, void*);
            return convert_pointer_to_string(ptr, alloc);
        }
        default: {
            // Handle unknown format specifier
            char* result = (char*)sa_alloc(alloc, 2);
            result[0] = '%';
            result[1] = specifier;
            return result;
        }
    }
}

struct format_iterator {
    const char* format;
    const char* current;
    const char* segment_start;
    const char* segment_end;
    char specifier;
    u8 in_meta;
    const meta* meta;
    void* data;
    print_meta_iterator* meta_iter;
    stack_alloc* alloc;
    uptr offset;
    va_list args;
    char _buffer[1024];
    stack_alloc buffer;
};

format_iterator* format_iterator_init(stack_alloc* alloc, const char* format, va_list args) {
    format_iterator* iter = sa_alloc(alloc, sizeof(format_iterator));
    iter->format = format;
    iter->current = format;
    iter->segment_start = format;
    iter->segment_end = format;
    iter->specifier = 0;
    iter->in_meta = 0;
    iter->meta = NULL;
    iter->data = NULL;
    iter->meta_iter = NULL;
    iter->alloc = alloc;
    iter->offset = 0;
    va_copy(iter->args, args);
    sa_init(&iter->buffer, iter->_buffer, byteoffset(iter->_buffer, sizeof(iter->_buffer)));
    return iter;
}

void format_iterator_deinit(stack_alloc* alloc, format_iterator* iterator) {
    sa_deinit(&iterator->buffer);
    sa_free(alloc, iterator);
}



format_iteration format_iterator_next(format_iterator* iter) {
    iter->buffer.cursor = iter->buffer.begin;
    if (iter->in_meta) {
        print_meta_iteration result = print_meta_iterator_next(iter->meta_iter);
        if (!result.meta) {
            iter->in_meta = 0;
            print_meta_iterator_deinit(iter->alloc, iter->meta_iter);
            iter->meta_iter = NULL;
            // Continue with format
            return format_iterator_next(iter);
        }
        const meta* current = result.meta;
        if (current->pt != PT_NONE) {
            // Primitive
            void* data_offset = byteoffset(iter->data, iter->offset);
            char* result;
            switch (current->pt) {
                case PT_I8: result = convert_i8_to_string(*(i8*)data_offset, &iter->buffer); break;
                case PT_U8: result = convert_u8_to_string(*(u8*)data_offset, &iter->buffer); break;
                case PT_I16: result = convert_i16_to_string(*(i16*)data_offset, &iter->buffer); break;
                case PT_U16: result = convert_u16_to_string(*(u16*)data_offset, &iter->buffer); break;
                case PT_I32: result = convert_i32_to_string(*(i32*)data_offset, &iter->buffer); break;
                case PT_U32: result = convert_u32_to_string(*(u32*)data_offset, &iter->buffer); break;
                case PT_I64: result = convert_i64_to_string(*(i64*)data_offset, &iter->buffer); break;
                case PT_U64: result = convert_u64_to_string(*(u64*)data_offset, &iter->buffer); break;
                case PT_IPTR: result = convert_iptr_to_string(*(iptr*)data_offset, &iter->buffer); break;
                case PT_UPTR: result = convert_uptr_to_string(*(uptr*)data_offset, &iter->buffer); break;
                default: {
                    const string_span unknown = STR("<unknown primitive>");
                    result = (char*)sa_alloc(&iter->buffer, bytesize(unknown.begin, unknown.end));
                    sa_copy(&iter->buffer, unknown.begin, result, bytesize(unknown.begin, unknown.end));
                    break;
                }
                    
            }
            return (format_iteration){FORMAT_ITERATION_LITERAL, {result, iter->buffer.cursor}};
        } else {
            // Struct
            void* start = iter->buffer.cursor;
            if (result.fields_current == result.meta->fields.begin) {
                uptr name_len = bytesize(current->type_name.begin, current->type_name.end);
                void* cursor = sa_alloc(&iter->buffer, name_len);
                sa_copy(&iter->buffer, current->type_name.begin, cursor, name_len);
                cursor = sa_alloc(&iter->buffer, 2);
                sa_copy(&iter->buffer, " {", cursor, 2);
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                cursor = sa_alloc(&iter->buffer, field_len);
                sa_copy(&iter->buffer, result.fields_current->field_name.begin, cursor, field_len);
                cursor = sa_alloc(&iter->buffer, 2);
                sa_copy(&iter->buffer, ": ", cursor, 2);
                iter->offset += result.fields_current->offset;
            } else if (result.fields_current == result.meta->fields.end) {
                void* cursor = sa_alloc(&iter->buffer, 1);
                sa_copy(&iter->buffer, "}", cursor, 1);
                iter->offset -= (result.meta->fields.end - 1)->offset;
            } else {
                void* cursor = sa_alloc(&iter->buffer, 2);
                sa_copy(&iter->buffer, ", ", cursor, 2);
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                cursor = sa_alloc(&iter->buffer, field_len);
                sa_copy(&iter->buffer, result.fields_current->field_name.begin, cursor, field_len);;
                cursor = sa_alloc(&iter->buffer, 2);
                sa_copy(&iter->buffer, ": ", cursor, 2);
                iter->offset += result.fields_current->offset;
            }
            return (format_iteration){FORMAT_ITERATION_LITERAL, {start, iter->buffer.cursor}};
        }
    } else {
        if (*iter->current == '\0') {
            return (format_iteration){FORMAT_ITERATION_END, {NULL, 0}};
        }
        if (*iter->current == '%') {
            iter->segment_start = iter->current;
            iter->current++;
            if (*iter->current == '\0') {
                iter->segment_end = iter->current;
                iter->current++;
                return (format_iteration){FORMAT_ITERATION_LITERAL, {iter->segment_start, iter->segment_end}};
            }
            iter->specifier = *iter->current;
            iter->segment_end = iter->current + 1;
            iter->current++;
            if (iter->specifier == 'm') {
                iter->meta = va_arg(iter->args, const meta*);
                iter->data = va_arg(iter->args, void*);
                iter->in_meta = 1;
                iter->offset = 0;
                iter->meta_iter = print_meta_iterator_init(iter->alloc, iter->meta);
                return format_iterator_next(iter);
            } else {
                // Process the specifier
                char* result = process_format_specifier(iter->specifier, iter->args, &iter->buffer);
                return (format_iteration){FORMAT_ITERATION_LITERAL, {result, iter->buffer.cursor}};
            }
        } else {
            iter->segment_start = iter->current;
            while (*iter->current != '\0' && *iter->current != '%') {
                iter->current++;
            }
            iter->segment_end = iter->current;
            return (format_iteration){FORMAT_ITERATION_LITERAL, {iter->segment_start, iter->segment_end}};
        }
    }
}
