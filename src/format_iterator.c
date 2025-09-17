#include "format_iterator.h"
#include "convert.h"
#include "./meta_iterator.h"
#include "stack_alloc.h"
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

// Process a format specifier and get the result
static void process_format_specifier(char specifier, va_list args, stack_alloc* alloc, char* buffer, uptr* length) {
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
                sa_copy(alloc, str, buffer, *length);
            } else {
                // Copy "(null)" to buffer
                const char* null_str = "(null)";
                *length = 6;
                sa_copy(alloc, null_str, buffer, *length);
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
    uptr buffer_pos;
};

format_iterator* format_iterator_init(stack_alloc* alloc, const char* format, va_list args) {
    format_iterator* iter = sa_alloc(alloc, sizeof(format_iterator));
    iter->format = format;
    iter->current = format;
    iter->segment_start = format;
    iter->segment_end = format;
    iter->specifier = '\0';
    iter->in_meta = 0;
    iter->meta = NULL;
    iter->data = NULL;
    iter->meta_iter = NULL;
    iter->alloc = alloc;
    iter->offset = 0;
    va_copy(iter->args, args);
    sa_init(&iter->buffer, iter->_buffer, byteoffset(iter->_buffer, sizeof(iter->_buffer)));
    iter->buffer_pos = 0;
    return iter;
}

void format_iterator_deinit(stack_alloc* alloc, format_iterator* iterator) {
    sa_deinit(&iterator->buffer);
    sa_free(alloc, iterator);
}



format_iteration format_iterator_next(format_iterator* iter) {
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
            char _buf[256];
            stack_alloc buf;
            sa_init(&buf, _buf, byteoffset(_buf, sizeof(_buf)));
            uptr len;
            void* data_offset = byteoffset(iter->data, iter->offset);
            switch (current->pt) {
                case PT_I8: convert_i8_to_string(*(i8*)data_offset, _buf, &len); break;
                case PT_U8: convert_u8_to_string(*(u8*)data_offset, _buf, &len); break;
                case PT_I16: convert_i16_to_string(*(i16*)data_offset, _buf, &len); break;
                case PT_U16: convert_u16_to_string(*(u16*)data_offset, _buf, &len); break;
                case PT_I32: convert_i32_to_string(*(i32*)data_offset, _buf, &len); break;
                case PT_U32: convert_u32_to_string(*(u32*)data_offset, _buf, &len); break;
                case PT_I64: convert_i64_to_string(*(i64*)data_offset, _buf, &len); break;
                case PT_U64: convert_u64_to_string(*(u64*)data_offset, _buf, &len); break;
                case PT_IPTR: convert_iptr_to_string(*(iptr*)data_offset, _buf, &len); break;
                case PT_UPTR: convert_uptr_to_string(*(uptr*)data_offset, _buf, &len); break;
                default:
                    len = 18;
                    memcpy(_buf, "<unknown primitive>", len);
                    break;
            }
            uptr start = iter->buffer_pos;
            sa_copy(&iter->buffer, _buf, iter->_buffer + iter->buffer_pos, len);
            iter->buffer_pos += len;
            sa_deinit(&buf);
            return (format_iteration){FORMAT_ITERATION_LITERAL, iter->_buffer + start, len};
        } else {
            // Struct
            uptr start = iter->buffer_pos;
            if (result.fields_current == result.meta->fields.begin) {
                uptr name_len = bytesize(current->type_name.begin, current->type_name.end);
                sa_copy(&iter->buffer, current->type_name.begin, iter->_buffer + iter->buffer_pos, name_len);
                iter->buffer_pos += name_len;
                sa_copy(&iter->buffer, " {", iter->_buffer + iter->buffer_pos, 2);
                iter->buffer_pos += 2;
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                sa_copy(&iter->buffer, result.fields_current->field_name.begin, iter->_buffer + iter->buffer_pos, field_len);
                iter->buffer_pos += field_len;
                sa_copy(&iter->buffer, ": ", iter->_buffer + iter->buffer_pos, 2);
                iter->buffer_pos += 2;
                iter->offset += result.fields_current->offset;
            } else if (result.fields_current == result.meta->fields.end) {
                sa_copy(&iter->buffer, "}", iter->_buffer + iter->buffer_pos, 1);
                iter->buffer_pos += 1;
                iter->offset -= (result.meta->fields.end - 1)->offset;
            } else {
                sa_copy(&iter->buffer, ", ", iter->_buffer + iter->buffer_pos, 2);
                iter->buffer_pos += 2;
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                sa_copy(&iter->buffer, result.fields_current->field_name.begin, iter->_buffer + iter->buffer_pos, field_len);
                iter->buffer_pos += field_len;
                sa_copy(&iter->buffer, ": ", iter->_buffer + iter->buffer_pos, 2);
                iter->buffer_pos += 2;
                iter->offset += result.fields_current->offset;
            }
            uptr len = iter->buffer_pos - start;
            return (format_iteration){FORMAT_ITERATION_LITERAL, iter->_buffer + start, len};
        }
    } else {
        if (*iter->current == '\0') {
            return (format_iteration){FORMAT_ITERATION_END, NULL, 0};
        }
        if (*iter->current == '%') {
            iter->segment_start = iter->current;
            iter->current++;
            if (*iter->current == '\0') {
                iter->segment_end = iter->current;
                iter->current++;
                return (format_iteration){FORMAT_ITERATION_LITERAL, iter->segment_start, iter->segment_end - iter->segment_start};
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
                uptr length;
                uptr start = iter->buffer_pos;
                process_format_specifier(iter->specifier, iter->args, &iter->buffer, iter->_buffer + iter->buffer_pos, &length);
                iter->buffer_pos += length;
                return (format_iteration){FORMAT_ITERATION_LITERAL, iter->_buffer + start, length};
            }
        } else {
            iter->segment_start = iter->current;
            while (*iter->current != '\0' && *iter->current != '%') {
                iter->current++;
            }
            iter->segment_end = iter->current;
            return (format_iteration){FORMAT_ITERATION_LITERAL, iter->segment_start, iter->segment_end - iter->segment_start};
        }
    }
}
