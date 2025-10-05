
#include "format_iterator.h"
#include "convert.h"
#include "meta_iterator.h"
#include "stack_alloc.h"
#include "assert.h"
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

// Shared text buffer and allocator used across chained format_iterators
typedef struct {
    stack_alloc text_format_alloc;
} format_shared_text;

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

    u8 in_meta_array;
    const meta* meta_array;
    void* meta_array_data_to_format_begin;
    void* meta_array_data_to_format_end;
    void* meta_array_data_to_format_current;
    format_iterator* meta_array_iterator;
    
    stack_alloc* alloc;
    uptr offset;
    va_list args;

    // State for handling large strings in chunks
    u8 in_string;
    const char* string_current;
    const char* string_end;

    // Shared temporary buffer management for all stacked iterators in a chain
    format_shared_text* format_shared_text;
    u8 owns_format_shared_text;
};

static format_iterator* format_iterator_init_extented(
        stack_alloc* alloc, 
        string format, va_list args, 
        format_shared_text* format_shared_text,
        u8 owns_format_shared_text
) {
    format_iterator* iter = sa_alloc(alloc, sizeof(format_iterator));
    iter->format = format;
    iter->format_current = format.begin;
    iter->format_segment.begin = format.begin;
    iter->format_segment.end = format.begin;
    iter->specifier = 0;
    iter->in_meta = 0;
    iter->meta = 0;
    iter->data_to_format = 0;
    iter->meta_iter = 0;

    iter->in_meta_array = 0;
    iter->meta_array = 0;
    iter->meta_array_data_to_format_begin = 0;
    iter->meta_array_data_to_format_end = 0;
    iter->meta_array_data_to_format_current = 0;
    iter->meta_array_iterator = 0;

    iter->alloc = alloc;
    iter->offset = 0;
    iter->in_string = 0;
    iter->string_current = 0;
    iter->string_end = 0;

    va_copy(iter->args, args);

    iter->format_shared_text = format_shared_text;
    iter->owns_format_shared_text = owns_format_shared_text;
    return iter;
}

// [TASK] This doesn't work properly when adding array token litterals? "[", ",", "]"
static format_iterator* format_iterator_init_wrapper_shared(format_iterator* parent, string format, ...) {
    va_list args;
    va_start(args, format);
    format_iterator* iterator = format_iterator_init_extented(parent->alloc, format, args, 
            parent->format_shared_text, /*owns_format_shared_text=*/ 0);
    va_end(args);
    return iterator;
}

format_iterator* format_iterator_init(stack_alloc* alloc, string format, va_list args) {
    
    format_shared_text* shared_text = sa_alloc(alloc, sizeof(*shared_text));
    void* begin = sa_alloc(alloc, 1024);
    void* end = alloc->cursor;
    sa_init(&shared_text->text_format_alloc, begin, end);
    
    return format_iterator_init_extented(alloc, format, args, shared_text, /*owns_format_shared_text=*/ 1);
}

void format_iterator_deinit(stack_alloc* alloc, format_iterator* iterator) {
    format_shared_text* format_shared_text = iterator->format_shared_text;
    
    sa_free(alloc, iterator);
    if (iterator->owns_format_shared_text) {
        sa_deinit(&format_shared_text->text_format_alloc);
        sa_free(alloc, format_shared_text);
    } else {
        debug_assert(format_shared_text->text_format_alloc.begin == format_shared_text->text_format_alloc.cursor);
    }
}

format_iteration format_iterator_next(format_iterator* iter) {
    stack_alloc* text_format_alloc = &iter->format_shared_text->text_format_alloc;
    text_format_alloc->cursor = text_format_alloc->begin;
    if (iter->in_meta_array) {
        if (iter->meta_array_iterator != 0) {
            format_iteration iteration = format_iterator_next(iter->meta_array_iterator);
            if (iteration.type == FORMAT_ITERATION_END) {
                format_iterator_deinit(iter->alloc, iter->meta_array_iterator);
                iter->meta_array_iterator = 0;
                return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
            }
            return iteration;
        } else {
            const meta* element_meta = iter->meta_array->array_element_meta;
            if (iter->meta_array_data_to_format_current == iter->meta_array_data_to_format_begin) {
                // First
                iter->meta_array_iterator = format_iterator_init_wrapper_shared(iter, STRING("%m"), element_meta, iter->meta_array_data_to_format_current);
                iter->meta_array_data_to_format_current = byteoffset(iter->meta_array_data_to_format_current, element_meta->type_size);
                /*[TASK] Write "[" litteral*/
                return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
            } else if(iter->meta_array_data_to_format_current == iter->meta_array_data_to_format_end) {
                iter->in_meta_array = 0;
                /*[TASK] Write "]" litteral*/
                return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
            } else {
                iter->meta_array_iterator = format_iterator_init_wrapper_shared(iter, STRING("%m"), element_meta, iter->meta_array_data_to_format_current);
                iter->meta_array_data_to_format_current = byteoffset(iter->meta_array_data_to_format_current, element_meta->type_size);
                /*[TASK] Write ", " litteral*/
                return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
            }
        }
    } else if (iter->in_meta) {
        print_meta_iteration result = print_meta_iterator_next(iter->meta_iter);
        if (!result.meta) {
            iter->in_meta = 0;
            print_meta_iterator_deinit(iter->alloc, iter->meta_iter);
            iter->meta_iter = 0;
            // Continue with format
            return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
        }
        const meta* current = result.meta;
        if (current->pt != PT_NONE) {
            if (current->pt == PT_ARRAY) {
                iter->in_meta_array = 1;
                iter->meta_array = current;
                iter->meta_array_data_to_format_begin = *(void**)(byteoffset(iter->data_to_format, iter->offset));
                iter->meta_array_data_to_format_end = *(void**)(byteoffset(iter->data_to_format, iter->offset + sizeof(void*)));
                iter->meta_array_data_to_format_current = iter->meta_array_data_to_format_begin;
                return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
            }
            // Primitive
            void* data_offset = byteoffset(iter->data_to_format, iter->offset);
            char* result;
            switch (current->pt) {
                case PT_I8: result = convert_i8_to_string(*(i8*)data_offset, text_format_alloc); break;
                case PT_U8: result = convert_u8_to_string(*(u8*)data_offset, text_format_alloc); break;
                case PT_I16: result = convert_i16_to_string(*(i16*)data_offset, text_format_alloc); break;
                case PT_U16: result = convert_u16_to_string(*(u16*)data_offset, text_format_alloc); break;
                case PT_I32: result = convert_i32_to_string(*(i32*)data_offset, text_format_alloc); break;
                case PT_U32: result = convert_u32_to_string(*(u32*)data_offset, text_format_alloc); break;
                case PT_I64: result = convert_i64_to_string(*(i64*)data_offset, text_format_alloc); break;
                case PT_U64: result = convert_u64_to_string(*(u64*)data_offset, text_format_alloc); break;
                case PT_IPTR: result = convert_iptr_to_string(*(iptr*)data_offset, text_format_alloc); break;
                case PT_UPTR: result = convert_uptr_to_string(*(uptr*)data_offset, text_format_alloc); break;
                default: {
                    const string unknown = STR("<unknown primitive>");
                    result = (char*)sa_alloc(text_format_alloc, bytesize(unknown.begin, unknown.end));
                    sa_copy(text_format_alloc, unknown.begin, result, bytesize(unknown.begin, unknown.end));
                    break;
                }
                
            }
            return (format_iteration){FORMAT_ITERATION_LITERAL, {result, text_format_alloc->cursor}};
        } else {
            // Struct
            void* start = text_format_alloc->cursor;
            if (result.fields_current == result.meta->fields.begin) {
                uptr name_len = bytesize(current->type_name.begin, current->type_name.end);
                void* cursor = sa_alloc(text_format_alloc, name_len);
                sa_copy(text_format_alloc, current->type_name.begin, cursor, name_len);
                cursor = sa_alloc(text_format_alloc, 2);
                sa_copy(text_format_alloc, " {", cursor, 2);
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                cursor = sa_alloc(text_format_alloc, field_len);
                sa_copy(text_format_alloc, result.fields_current->field_name.begin, cursor, field_len);
                cursor = sa_alloc(text_format_alloc, 2);
                sa_copy(text_format_alloc, ": ", cursor, 2);
                iter->offset = result.fields_current->offset;
            } else if (result.fields_current == result.meta->fields.end) {
                void* cursor = sa_alloc(text_format_alloc, 1);
                sa_copy(text_format_alloc, "}", cursor, 1);
                iter->offset -= (result.meta->fields.end - 1)->offset;
            } else {
                void* cursor = sa_alloc(text_format_alloc, 2);
                sa_copy(text_format_alloc, ", ", cursor, 2);
                uptr field_len = bytesize(result.fields_current->field_name.begin, result.fields_current->field_name.end);
                cursor = sa_alloc(text_format_alloc, field_len);
                sa_copy(text_format_alloc, result.fields_current->field_name.begin, cursor, field_len);;
                cursor = sa_alloc(text_format_alloc, 2);
                sa_copy(text_format_alloc, ": ", cursor, 2);
                iter->offset = result.fields_current->offset;
            }
            return (format_iteration){FORMAT_ITERATION_LITERAL, {start, text_format_alloc->cursor}};
        }
    } else if (iter->in_string) {
        if (iter->string_current == iter->string_end) {
            iter->in_string = 0;
            return (format_iteration){FORMAT_ITERATION_CONTINUE, {0,0}};
        }
        uptr remaining = (uptr)(iter->string_end - iter->string_current);
        uptr available = (uptr)((char*)iter->format_shared_text->text_format_alloc.end - (char*)text_format_alloc->cursor);
        uptr chunk_size = remaining < available ? remaining : available;
        char* result = (char*)sa_alloc(text_format_alloc, chunk_size);
        sa_copy(text_format_alloc, iter->string_current, result, chunk_size);
        iter->string_current += chunk_size;
        return (format_iteration){FORMAT_ITERATION_LITERAL, {result, text_format_alloc->cursor}};
    } else {
        if (iter->format_current == iter->format.end) {
            return (format_iteration){FORMAT_ITERATION_END, {0, 0}};
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
                    char* result = sa_alloc(text_format_alloc, null_str_size);
                    sa_copy(text_format_alloc, null_str.begin, result, null_str_size);
                    return (format_iteration){FORMAT_ITERATION_LITERAL, {result, text_format_alloc->cursor}};
                }
            } else {
                // Process other specifiers
                char* result = process_format_specifier(iter->specifier, iter->args, text_format_alloc);
                return (format_iteration){FORMAT_ITERATION_LITERAL, {result, text_format_alloc->cursor}};
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
