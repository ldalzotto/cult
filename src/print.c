#include "print.h"
#include <stddef.h>  // for NULL


// Helper function to print indentation
static void print_indent(FILE* stream, u32 indent_level) {
    for (u32 i = 0; i < indent_level; ++i) {
        fprintf(stream, "  ");
    }
}

// Generic print function
void print_generic(const print_meta* meta, void* data, FILE* stream, u32 indent_level) {
    if (meta->pt != PT_NONE) {
        // Primitive
        switch (meta->pt) {
            case PT_I8: fprintf(stream, "%d", *(i8*)data); break;
            case PT_U8: fprintf(stream, "%u", *(u8*)data); break;
            case PT_I16: fprintf(stream, "%d", *(i16*)data); break;
            case PT_U16: fprintf(stream, "%u", *(u16*)data); break;
            case PT_I32: fprintf(stream, "%d", *(i32*)data); break;
            case PT_U32: fprintf(stream, "%u", *(u32*)data); break;
            case PT_I64: fprintf(stream, "%lld", *(i64*)data); break;
            case PT_U64: fprintf(stream, "%llu", *(u64*)data); break;
            case PT_IPTR: fprintf(stream, "%ld", *(iptr*)data); break;
            case PT_UPTR: fprintf(stream, "%lu", *(uptr*)data); break;
            default: fprintf(stream, "<unknown primitive>"); break;
        }
    } else {
        // Struct
        fprintf(stream, "%s {\n", meta->type_name);
        field_descriptor* field = (field_descriptor*)meta->fields_begin;
        field_descriptor* field_end = (field_descriptor*)meta->fields_end;
        for (; field < field_end; ++field) {
            print_indent(stream, indent_level + 1);
            fprintf(stream, "%s: ", field->field_name);
            void* field_data = byteoffset(data, field->offset);
            print_generic(field->field_meta, field_data, stream, indent_level + 1);
            fprintf(stream, "\n");
        }
        print_indent(stream, indent_level);
        fprintf(stream, "}");
    }
}

// Specialized function for printing arrays
void print_array_generic(const print_meta* element_meta, void* begin, void* end, FILE* stream, u32 indent_level) {
    fprintf(stream, "[");
    if (begin < end) {
        void* current = begin;
        print_generic(element_meta, current, stream, indent_level);
        current = byteoffset(current, element_meta->type_size);
        while (current < end) {
            fprintf(stream, ", ");
            print_generic(element_meta, current, stream, indent_level);
            current = byteoffset(current, element_meta->type_size);
        }
    }
    fprintf(stream, "]");
}
