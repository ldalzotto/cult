#ifndef GEOM_PRIMITIVE_H
#define GEOM_PRIMITIVE_H

#include "../primitive.h"

// 2D vector with i32 components
typedef struct {
    i32 x, y;
} vec2_i32;

// 3D vector with i32 components
typedef struct {
    i32 x, y, z;
} vec3_i32;

// 4D vector with i32 components
typedef struct {
    i32 x, y, z, w;
} vec4_i32;

// 3x3 matrix with i32 components (stored column-major)
typedef struct {
    i32 m[9];
} mat3_i32;

// 4x4 matrix with i32 components (stored column-major)
typedef struct {
    i32 m[16];
} mat4_i32;

// Vector operations
vec2_i32 vec2_i32_add(vec2_i32 a, vec2_i32 b);
vec3_i32 vec3_i32_add(vec3_i32 a, vec3_i32 b);
vec4_i32 vec4_i32_add(vec4_i32 a, vec4_i32 b);

vec2_i32 vec2_i32_sub(vec2_i32 a, vec2_i32 b);
vec3_i32 vec3_i32_sub(vec3_i32 a, vec3_i32 b);
vec4_i32 vec4_i32_sub(vec4_i32 a, vec4_i32 b);

vec2_i32 vec2_i32_mul_scalar(vec2_i32 v, i32 s);
vec3_i32 vec3_i32_mul_scalar(vec3_i32 v, i32 s);
vec4_i32 vec4_i32_mul_scalar(vec4_i32 v, i32 s);

i64 vec2_i32_dot(vec2_i32 a, vec2_i32 b);
i64 vec3_i32_dot(vec3_i32 a, vec3_i32 b);
i64 vec4_i32_dot(vec4_i32 a, vec4_i32 b);

vec3_i32 vec3_i32_cross(vec3_i32 a, vec3_i32 b);

// Matrix operations
mat3_i32 mat3_i32_mul(mat3_i32 a, mat3_i32 b);
mat4_i32 mat4_i32_mul(mat4_i32 a, mat4_i32 b);

vec3_i32 mat3_i32_mul_vec3(mat3_i32 m, vec3_i32 v);
vec4_i32 mat4_i32_mul_vec4(mat4_i32 m, vec4_i32 v);

// Identity matrices
mat3_i32 mat3_i32_identity(void);
mat4_i32 mat4_i32_identity(void);

// Basic transformations
mat4_i32 mat4_i32_translate(i32 x, i32 y, i32 z);
mat4_i32 mat4_i32_scale(i32 x, i32 y, i32 z);

#endif /* GEOM_PRIMITIVE_H */
