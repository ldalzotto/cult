#ifndef LITTERAL_H
#define LITTERAL_H

// String span structure
typedef struct {
    const void* begin;
    const void* end;
} string_span;

// Macro to create a string_span from a static string literal
#define STATIC_STRING(str) {(str), (str) + sizeof(str) - 1}

#define ARRAY_RANGE(variable) \
      variable, byteoffset(variable, sizeof(variable))

#endif /* LITTERAL_H */
