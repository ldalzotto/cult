#include "format_iterator.h"
#include "convert.h"
#include "./assert.h"
#include "./print_meta_iterator.h"
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

struct format_iterator {
    const char* format;
    const char* current;
    const char* segment_start;
    const char* segment_end;
    char specifier;
    u8 in_meta;
    const print_meta* meta;
    void* data;
    print_meta_iterator* meta_iter;
    stack_alloc* alloc;
    uptr offset;
    va_list args;
    char buffer[1024];
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
    return iter;
}

void format_iterator_deinit(stack_alloc* alloc, format_iterator* iterator) {
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
        const print_meta* current = result.meta;
        if (current->pt != PT_NONE) {
            // Primitive
            char buf[256];
            uptr len;
            void* data_offset = byteoffset(iter->data, iter->offset);
            switch (current->pt) {
                case PT_I8: convert_i8_to_string(*(i8*)data_offset, buf, &len); break;
                case PT_U8: convert_u8_to_string(*(u8*)data_offset, buf, &len); break;
                case PT_I16: convert_i16_to_string(*(i16*)data_offset, buf, &len); break;
                case PT_U16: convert_u16_to_string(*(u16*)data_offset, buf, &len); break;
                case PT_I32: convert_i32_to_string(*(i32*)data_offset, buf, &len); break;
                case PT_U32: convert_u32_to_string(*(u32*)data_offset, buf, &len); break;
                case PT_I64: convert_i64_to_string(*(i64*)data_offset, buf, &len); break;
                case PT_U64: convert_u64_to_string(*(u64*)data_offset, buf, &len); break;
                case PT_IPTR: convert_iptr_to_string(*(iptr*)data_offset, buf, &len); break;
                case PT_UPTR: convert_uptr_to_string(*(uptr*)data_offset, buf, &len); break;
                default:
                    len = 18;
                    memcpy(buf, "<unknown primitive>", len);
                    break;
            }
            memcpy(iter->buffer, buf, len);
            return (format_iteration){FORMAT_ITERATION_LITERAL, iter->buffer, len, '\0'};
        } else {
            // Struct
            uptr pos = 0;
            if (result.fields_current == result.meta->fields_begin) {
                uptr name_len = bytesize(current->type_name.begin, current->type_name.end);
                memcpy(iter->buffer + pos, current->type_name.begin, name_len);
                pos += name_len;
                memcpy(iter->buffer + pos, " {", 2);
                pos += 2;
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                memcpy(iter->buffer + pos, result.fields_current->field_name.begin, field_len);
                pos += field_len;
                memcpy(iter->buffer + pos, ": ", 2);
                pos += 2;
                iter->offset += result.fields_current->offset;
            } else if (result.fields_current == result.meta->fields_end) {
                memcpy(iter->buffer + pos, "}", 1);
                pos = 1;
                iter->offset -= (result.meta->fields_end - 1)->offset;
            } else {
                memcpy(iter->buffer + pos, ", ", 2);
                pos += 2;
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                memcpy(iter->buffer + pos, result.fields_current->field_name.begin, field_len);
                pos += field_len;
                memcpy(iter->buffer + pos, ": ", 2);
                pos += 2;
                iter->offset += result.fields_current->offset;
            }
            return (format_iteration){FORMAT_ITERATION_LITERAL, iter->buffer, pos, '\0'};
        }
    } else {
        if (*iter->current == '\0') {
            return (format_iteration){FORMAT_ITERATION_END, NULL, 0, '\0'};
        }
        if (*iter->current == '%') {
            iter->segment_start = iter->current;
            iter->current++;
            if (*iter->current == '\0') {
                iter->segment_end = iter->current;
                iter->current++;
                return (format_iteration){FORMAT_ITERATION_LITERAL, iter->segment_start, iter->segment_end - iter->segment_start, '\0'};
            }
            iter->specifier = *iter->current;
            iter->segment_end = iter->current + 1;
            iter->current++;
            if (iter->specifier == 'm') {
                iter->meta = va_arg(iter->args, const print_meta*);
                iter->data = va_arg(iter->args, void*);
                iter->in_meta = 1;
                iter->offset = 0;
                iter->meta_iter = print_meta_iterator_init(iter->alloc, iter->meta);
                return format_iterator_next(iter);
            } else {
                return (format_iteration){FORMAT_ITERATION_SPECIFIER, iter->segment_start, iter->segment_end - iter->segment_start, iter->specifier};
            }
        } else {
            iter->segment_start = iter->current;
            while (*iter->current != '\0' && *iter->current != '%') {
                iter->current++;
            }
            iter->segment_end = iter->current;
            return (format_iteration){FORMAT_ITERATION_LITERAL, iter->segment_start, iter->segment_end - iter->segment_start, '\0'};
        }
    }
}
