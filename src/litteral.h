#ifndef LITTERAL_H
#define LITTERAL_H

// String span structure
typedef struct {
    const void* begin;
    const void* end;
} string;

// Macro to create a string_span from a static string literal
#define STR(str) {(str), (str) + sizeof(str) - 1}
#define STRING(str) (string) STR(str)

#define RANGE(variable) \
      variable, byteoffset(variable, sizeof(variable))

#endif /* LITTERAL_H */
