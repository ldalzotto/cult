#include "primitive.h"

// Vector addition
vec2_i32 vec2_i32_add(vec2_i32 a, vec2_i32 b) {
    return (vec2_i32){a.x + b.x, a.y + b.y};
}

vec3_i32 vec3_i32_add(vec3_i32 a, vec3_i32 b) {
    return (vec3_i32){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec4_i32 vec4_i32_add(vec4_i32 a, vec4_i32 b) {
    return (vec4_i32){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

// Vector subtraction
vec2_i32 vec2_i32_sub(vec2_i32 a, vec2_i32 b) {
    return (vec2_i32){a.x - b.x, a.y - b.y};
}

vec3_i32 vec3_i32_sub(vec3_i32 a, vec3_i32 b) {
    return (vec3_i32){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec4_i32 vec4_i32_sub(vec4_i32 a, vec4_i32 b) {
    return (vec4_i32){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

// Scalar multiplication
vec2_i32 vec2_i32_mul_scalar(vec2_i32 v, i32 s) {
    return (vec2_i32){v.x * s, v.y * s};
}

vec3_i32 vec3_i32_mul_scalar(vec3_i32 v, i32 s) {
    return (vec3_i32){v.x * s, v.y * s, v.z * s};
}

vec4_i32 vec4_i32_mul_scalar(vec4_i32 v, i32 s) {
    return (vec4_i32){v.x * s, v.y * s, v.z * s, v.w * s};
}

// Dot product (returns i64 to handle overflow)
i64 vec2_i32_dot(vec2_i32 a, vec2_i32 b) {
    return (i64)a.x * b.x + (i64)a.y * b.y;
}

i64 vec3_i32_dot(vec3_i32 a, vec3_i32 b) {
    return (i64)a.x * b.x + (i64)a.y * b.y + (i64)a.z * b.z;
}

i64 vec4_i32_dot(vec4_i32 a, vec4_i32 b) {
    return (i64)a.x * b.x + (i64)a.y * b.y + (i64)a.z * b.z + (i64)a.w * b.w;
}

// Cross product
vec3_i32 vec3_i32_cross(vec3_i32 a, vec3_i32 b) {
    return (vec3_i32){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// Matrix multiplication
mat3_i32 mat3_i32_mul(mat3_i32 a, mat3_i32 b) {
    mat3_i32 result;
    for (i32 i = 0; i < 3; ++i) {
        for (i32 j = 0; j < 3; ++j) {
            i64 sum = 0;
            for (i32 k = 0; k < 3; ++k) {
                sum += (i64)a.m[i * 3 + k] * b.m[k * 3 + j];
            }
            result.m[i * 3 + j] = (i32)sum; // Truncate to i32
        }
    }
    return result;
}

mat4_i32 mat4_i32_mul(mat4_i32 a, mat4_i32 b) {
    mat4_i32 result;
    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            i64 sum = 0;
            for (i32 k = 0; k < 4; ++k) {
                sum += (i64)a.m[i * 4 + k] * b.m[k * 4 + j];
            }
            result.m[i * 4 + j] = (i32)sum; // Truncate to i32
        }
    }
    return result;
}

// Matrix-vector multiplication
vec3_i32 mat3_i32_mul_vec3(mat3_i32 m, vec3_i32 v) {
    return (vec3_i32){
        (i32)((i64)m.m[0] * v.x + (i64)m.m[3] * v.y + (i64)m.m[6] * v.z),
        (i32)((i64)m.m[1] * v.x + (i64)m.m[4] * v.y + (i64)m.m[7] * v.z),
        (i32)((i64)m.m[2] * v.x + (i64)m.m[5] * v.y + (i64)m.m[8] * v.z)
    };
}

vec4_i32 mat4_i32_mul_vec4(mat4_i32 m, vec4_i32 v) {
    return (vec4_i32){
        (i32)((i64)m.m[0] * v.x + (i64)m.m[4] * v.y + (i64)m.m[8] * v.z + (i64)m.m[12] * v.w),
        (i32)((i64)m.m[1] * v.x + (i64)m.m[5] * v.y + (i64)m.m[9] * v.z + (i64)m.m[13] * v.w),
        (i32)((i64)m.m[2] * v.x + (i64)m.m[6] * v.y + (i64)m.m[10] * v.z + (i64)m.m[14] * v.w),
        (i32)((i64)m.m[3] * v.x + (i64)m.m[7] * v.y + (i64)m.m[11] * v.z + (i64)m.m[15] * v.w)
    };
}

// Identity matrices
mat3_i32 mat3_i32_identity(void) {
    return (mat3_i32){{
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    }};
}

mat4_i32 mat4_i32_identity(void) {
    return (mat4_i32){{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }};
}

// Translation matrix
mat4_i32 mat4_i32_translate(i32 x, i32 y, i32 z) {
    mat4_i32 m = mat4_i32_identity();
    m.m[3] = x;
    m.m[7] = y;
    m.m[11] = z;
    return m;
}

// Scale matrix
mat4_i32 mat4_i32_scale(i32 x, i32 y, i32 z) {
    return (mat4_i32){{
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    }};
}
