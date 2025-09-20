#include "format_iterator.h"
#include "convert.h"
#include "meta_iterator.h"
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
    string format;
    struct {
        const char* begin;
        const char* end;
    } format_segment;
    const char* format_current;
    char specifier;
    u8 in_meta;
    const meta* meta;
    void* data_to_format;
    print_meta_iterator* meta_iter;
    stack_alloc* alloc;
    uptr offset;
    va_list args;

    // State for handling large strings in chunks
    u8 in_string;
    const char* string_current;
    const char* string_end;

    // A very small buffer that holds temporary data to be printed for each iteration
    char _text_format_buffer[1024];
    stack_alloc text_format_alloc;
};

format_iterator* format_iterator_init(stack_alloc* alloc, string format, va_list args) {
    format_iterator* iter = sa_alloc(alloc, sizeof(format_iterator));
    iter->format = format;
    iter->format_current = format.begin;
    iter->format_segment.begin = format.begin;
    iter->format_segment.end = format.begin;
    iter->specifier = 0;
    iter->in_meta = 0;
    iter->meta = NULL;
    iter->data_to_format = NULL;
    iter->meta_iter = NULL;
    iter->alloc = alloc;
    iter->offset = 0;
    iter->in_string = 0;
    iter->string_current = NULL;
    iter->string_end = NULL;
    va_copy(iter->args, args);
    sa_init(&iter->text_format_alloc, iter->_text_format_buffer, byteoffset(iter->_text_format_buffer, sizeof(iter->_text_format_buffer)));
    return iter;
}

void format_iterator_deinit(stack_alloc* alloc, format_iterator* iterator) {
    sa_deinit(&iterator->text_format_alloc);
    sa_free(alloc, iterator);
}

format_iteration format_iterator_next(format_iterator* iter) {
    iter->text_format_alloc.cursor = iter->text_format_alloc.begin;
    if (iter->in_meta) {
        print_meta_iteration result = print_meta_iterator_next(iter->meta_iter);
        if (!result.meta) {
            iter->in_meta = 0;
            print_meta_iterator_deinit(iter->alloc, iter->meta_iter);
            iter->meta_iter = NULL;
            // Continue with format
            return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
        }
        const meta* current = result.meta;
        if (current->pt != PT_NONE) {
            // Primitive
            void* data_offset = byteoffset(iter->data_to_format, iter->offset);
            char* result;
            switch (current->pt) {
                case PT_I8: result = convert_i8_to_string(*(i8*)data_offset, &iter->text_format_alloc); break;
                case PT_U8: result = convert_u8_to_string(*(u8*)data_offset, &iter->text_format_alloc); break;
                case PT_I16: result = convert_i16_to_string(*(i16*)data_offset, &iter->text_format_alloc); break;
                case PT_U16: result = convert_u16_to_string(*(u16*)data_offset, &iter->text_format_alloc); break;
                case PT_I32: result = convert_i32_to_string(*(i32*)data_offset, &iter->text_format_alloc); break;
                case PT_U32: result = convert_u32_to_string(*(u32*)data_offset, &iter->text_format_alloc); break;
                case PT_I64: result = convert_i64_to_string(*(i64*)data_offset, &iter->text_format_alloc); break;
                case PT_U64: result = convert_u64_to_string(*(u64*)data_offset, &iter->text_format_alloc); break;
                case PT_IPTR: result = convert_iptr_to_string(*(iptr*)data_offset, &iter->text_format_alloc); break;
                case PT_UPTR: result = convert_uptr_to_string(*(uptr*)data_offset, &iter->text_format_alloc); break;
                default: {
                    const string unknown = STR("<unknown primitive>");
                    result = (char*)sa_alloc(&iter->text_format_alloc, bytesize(unknown.begin, unknown.end));
                    sa_copy(&iter->text_format_alloc, unknown.begin, result, bytesize(unknown.begin, unknown.end));
                    break;
                }
                    
            }
            return (format_iteration){FORMAT_ITERATION_LITERAL, {result, iter->text_format_alloc.cursor}};
        } else {
            // Struct
            void* start = iter->text_format_alloc.cursor;
            if (result.fields_current == result.meta->fields.begin) {
                uptr name_len = bytesize(current->type_name.begin, current->type_name.end);
                void* cursor = sa_alloc(&iter->text_format_alloc, name_len);
                sa_copy(&iter->text_format_alloc, current->type_name.begin, cursor, name_len);
                cursor = sa_alloc(&iter->text_format_alloc, 2);
                sa_copy(&iter->text_format_alloc, " {", cursor, 2);
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                cursor = sa_alloc(&iter->text_format_alloc, field_len);
                sa_copy(&iter->text_format_alloc, result.fields_current->field_name.begin, cursor, field_len);
                cursor = sa_alloc(&iter->text_format_alloc, 2);
                sa_copy(&iter->text_format_alloc, ": ", cursor, 2);
                iter->offset += result.fields_current->offset;
            } else if (result.fields_current == result.meta->fields.end) {
                void* cursor = sa_alloc(&iter->text_format_alloc, 1);
                sa_copy(&iter->text_format_alloc, "}", cursor, 1);
                iter->offset -= (result.meta->fields.end - 1)->offset;
            } else {
                void* cursor = sa_alloc(&iter->text_format_alloc, 2);
                sa_copy(&iter->text_format_alloc, ", ", cursor, 2);
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                cursor = sa_alloc(&iter->text_format_alloc, field_len);
                sa_copy(&iter->text_format_alloc, result.fields_current->field_name.begin, cursor, field_len);;
                cursor = sa_alloc(&iter->text_format_alloc, 2);
                sa_copy(&iter->text_format_alloc, ": ", cursor, 2);
                iter->offset += result.fields_current->offset;
            }
            return (format_iteration){FORMAT_ITERATION_LITERAL, {start, iter->text_format_alloc.cursor}};
        }
    } else if (iter->in_string) {
        if (iter->string_current == iter->string_end) {
            iter->in_string = 0;
            return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
        }
        uptr remaining = (uptr)(iter->string_end - iter->string_current);
        uptr available = (uptr)((char*)iter->text_format_alloc.end - (char*)iter->text_format_alloc.cursor);
        uptr chunk_size = remaining < available ? remaining : available;
        char* result = (char*)sa_alloc(&iter->text_format_alloc, chunk_size);
        sa_copy(&iter->text_format_alloc, iter->string_current, result, chunk_size);
        iter->string_current += chunk_size;
        return (format_iteration){FORMAT_ITERATION_LITERAL, {result, iter->text_format_alloc.cursor}};
    } else {
        if (iter->format_current == iter->format.end) {
            return (format_iteration){FORMAT_ITERATION_END, {NULL, 0}};
        }
        if (*iter->format_current == '%') {
            iter->format_segment.begin = iter->format_current;
            iter->format_current++;
            if (iter->format_current == iter->format.end) {
                iter->format_segment.end = iter->format_current;
                iter->format_current++;
                return (format_iteration){FORMAT_ITERATION_LITERAL, {iter->format_segment.begin, iter->format_segment.end}};
            }
            iter->specifier = *iter->format_current;
            iter->format_segment.end = iter->format_current + 1;
            iter->format_current++;
            if (iter->specifier == 'm') {
                iter->meta = va_arg(iter->args, const meta*);
                iter->data_to_format = va_arg(iter->args, void*);
                iter->in_meta = 1;
                iter->offset = 0;
                iter->meta_iter = print_meta_iterator_init(iter->alloc, iter->meta);
                return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
            } else if (iter->specifier == 's') {
                // Handle string specially for chunking
                const string str = va_arg(iter->args, const string);
                if (str.begin && str.end) {
                    iter->in_string = 1;
                    iter->string_current = str.begin;
                    iter->string_end = str.end;
                    return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
                } else {
                    // Handle null string
                    const string null_str = STR("(null)");
                    const uptr null_str_size = bytesize(null_str.begin, null_str.end);
                    char* result = sa_alloc(&iter->text_format_alloc, null_str_size);
                    sa_copy(&iter->text_format_alloc, null_str.begin, result, null_str_size);
                    return (format_iteration){FORMAT_ITERATION_LITERAL, {result, iter->text_format_alloc.cursor}};
                }
            } else {
                // Process other specifiers
                char* result = process_format_specifier(iter->specifier, iter->args, &iter->text_format_alloc);
                return (format_iteration){FORMAT_ITERATION_LITERAL, {result, iter->text_format_alloc.cursor}};
            }
        } else {
            iter->format_segment.begin = iter->format_current;
            while (iter->format_current != iter->format.end && *iter->format_current != '%') {
                iter->format_current++;
            }
            iter->format_segment.end = iter->format_current;
            return (format_iteration){FORMAT_ITERATION_LITERAL, {iter->format_segment.begin, iter->format_segment.end}};
        }
    }
}
