#ifndef PRIMITIVE_H
#define PRIMITIVE_H

// Custom static assert macro for size verification
#define STATIC_ASSERT(expr) typedef char static_assert_array ## __LINE__ [(expr) ? 1 : -1]

// Primitive integer type definitions based on size assumptions:
// - char: 8 bits (guaranteed)
// - short: 16 bits (commonly reliable)
// - int: 32 bits (commonly reliable)
// - long long: 64 bits (guaranteed minimum)
// - long: pointer-sized (commonly reliable)

// Signed integer types
typedef signed char          i8;
typedef signed short         i16;
typedef signed int           i32;
typedef signed long          imax;
typedef signed long long     i64;

// Unsigned integer types
typedef unsigned char        u8;
typedef unsigned short       u16;
typedef unsigned int         u32;
typedef unsigned long        uimax;
typedef unsigned long long   u64;

// Size assertions
STATIC_ASSERT(sizeof(i8) == 1);
STATIC_ASSERT(sizeof(i16) == 2);
STATIC_ASSERT(sizeof(i32) == 4);
STATIC_ASSERT(sizeof(imax) == sizeof(void*));
STATIC_ASSERT(sizeof(i64) == 8);
STATIC_ASSERT(sizeof(u8) == 1);
STATIC_ASSERT(sizeof(u16) == 2);
STATIC_ASSERT(sizeof(u32) == 4);
STATIC_ASSERT(sizeof(uimax) == sizeof(void*));
STATIC_ASSERT(sizeof(u64) == 8);

#endif /* PRIMITIVE_H */
