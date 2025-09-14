#include "print.h"
#include <stddef.h>  // for NULL
#include <stdio.h>   // for sprintf

// Generic print function
void print_generic(const print_meta* meta, void* data, file_t file, u32 indent_level) {
    char buf[256];
    uptr len;
    if (meta->pt != PT_NONE) {
        // Primitive
        switch (meta->pt) {
            case PT_I8: len = sprintf(buf, "%d", *(i8*)data); break;
            case PT_U8: len = sprintf(buf, "%u", *(u8*)data); break;
            case PT_I16: len = sprintf(buf, "%d", *(i16*)data); break;
            case PT_U16: len = sprintf(buf, "%u", *(u16*)data); break;
            case PT_I32: len = sprintf(buf, "%d", *(i32*)data); break;
            case PT_U32: len = sprintf(buf, "%u", *(u32*)data); break;
            case PT_I64: len = sprintf(buf, "%lld", *(i64*)data); break;
            case PT_U64: len = sprintf(buf, "%llu", *(u64*)data); break;
            case PT_IPTR: len = sprintf(buf, "%ld", *(iptr*)data); break;
            case PT_UPTR: len = sprintf(buf, "%lu", *(uptr*)data); break;
            default: len = sprintf(buf, "<unknown primitive>"); break;
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
void print_string(const char* str, file_t file) {
    if (str) {
        uptr len = 0;
        while (str[len] != '\0') ++len;
        file_write(file, str, len);
    }
}
