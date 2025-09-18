
// Customizable aliases for geometry types
// Define macros before including this header to customize alias names.
// Example:
//   #define GEOM_VEC2_ALIAS my_vec2
//   #define GEOM_VEC3_ALIAS my_vec3
//   #include "geom/alias.h"
//
// If macros are not defined, no aliases are created.

#ifdef GEOM_VEC2_ALIAS
typedef vec2_i32 GEOM_VEC2_ALIAS;
#endif

#ifdef GEOM_VEC3_ALIAS
typedef vec3_i32 GEOM_VEC3_ALIAS;
#endif

#ifdef GEOM_VEC4_ALIAS
typedef vec4_i32 GEOM_VEC4_ALIAS;
#endif

#ifdef GEOM_MAT3_ALIAS
typedef mat3_i32 GEOM_MAT3_ALIAS;
#endif

#ifdef GEOM_MAT4_ALIAS
typedef mat4_i32 GEOM_MAT4_ALIAS;
#endif

// Function aliases as inline wrapper functions
// Define macros before including this header to create function aliases.
// Example:
//   #define GEOM_VEC2_ADD_ALIAS my_vec2_add
//   #include "geom/alias.h"
// Then my_vec2_add can be used as a direct function call.

#ifdef GEOM_VEC2_ADD_ALIAS
static inline vec2_i32 GEOM_VEC2_ADD_ALIAS(vec2_i32 a, vec2_i32 b) { return vec2_i32_add(a, b); }
#endif

#ifdef GEOM_VEC3_ADD_ALIAS
static inline vec3_i32 GEOM_VEC3_ADD_ALIAS(vec3_i32 a, vec3_i32 b) { return vec3_i32_add(a, b); }
#endif

#ifdef GEOM_VEC4_ADD_ALIAS
static inline vec4_i32 GEOM_VEC4_ADD_ALIAS(vec4_i32 a, vec4_i32 b) { return vec4_i32_add(a, b); }
#endif

#ifdef GEOM_VEC2_SUB_ALIAS
static inline vec2_i32 GEOM_VEC2_SUB_ALIAS(vec2_i32 a, vec2_i32 b) { return vec2_i32_sub(a, b); }
#endif

#ifdef GEOM_VEC3_SUB_ALIAS
static inline vec3_i32 GEOM_VEC3_SUB_ALIAS(vec3_i32 a, vec3_i32 b) { return vec3_i32_sub(a, b); }
#endif

#ifdef GEOM_VEC4_SUB_ALIAS
static inline vec4_i32 GEOM_VEC4_SUB_ALIAS(vec4_i32 a, vec4_i32 b) { return vec4_i32_sub(a, b); }
#endif

#ifdef GEOM_VEC2_MUL_SCALAR_ALIAS
static inline vec2_i32 GEOM_VEC2_MUL_SCALAR_ALIAS(vec2_i32 v, i32 s) { return vec2_i32_mul_scalar(v, s); }
#endif

#ifdef GEOM_VEC3_MUL_SCALAR_ALIAS
static inline vec3_i32 GEOM_VEC3_MUL_SCALAR_ALIAS(vec3_i32 v, i32 s) { return vec3_i32_mul_scalar(v, s); }
#endif

#ifdef GEOM_VEC4_MUL_SCALAR_ALIAS
static inline vec4_i32 GEOM_VEC4_MUL_SCALAR_ALIAS(vec4_i32 v, i32 s) { return vec4_i32_mul_scalar(v, s); }
#endif

#ifdef GEOM_VEC2_DOT_ALIAS
static inline i64 GEOM_VEC2_DOT_ALIAS(vec2_i32 a, vec2_i32 b) { return vec2_i32_dot(a, b); }
#endif

#ifdef GEOM_VEC3_DOT_ALIAS
static inline i64 GEOM_VEC3_DOT_ALIAS(vec3_i32 a, vec3_i32 b) { return vec3_i32_dot(a, b); }
#endif

#ifdef GEOM_VEC4_DOT_ALIAS
static inline i64 GEOM_VEC4_DOT_ALIAS(vec4_i32 a, vec4_i32 b) { return vec4_i32_dot(a, b); }
#endif

#ifdef GEOM_VEC3_CROSS_ALIAS
static inline vec3_i32 GEOM_VEC3_CROSS_ALIAS(vec3_i32 a, vec3_i32 b) { return vec3_i32_cross(a, b); }
#endif

#ifdef GEOM_MAT3_MUL_ALIAS
static inline mat3_i32 GEOM_MAT3_MUL_ALIAS(mat3_i32 a, mat3_i32 b) { return mat3_i32_mul(a, b); }
#endif

#ifdef GEOM_MAT4_MUL_ALIAS
static inline mat4_i32 GEOM_MAT4_MUL_ALIAS(mat4_i32 a, mat4_i32 b) { return mat4_i32_mul(a, b); }
#endif

#ifdef GEOM_MAT3_MUL_VEC3_ALIAS
static inline vec3_i32 GEOM_MAT3_MUL_VEC3_ALIAS(mat3_i32 m, vec3_i32 v) { return mat3_i32_mul_vec3(m, v); }
#endif

#ifdef GEOM_MAT4_MUL_VEC4_ALIAS
static inline vec4_i32 GEOM_MAT4_MUL_VEC4_ALIAS(mat4_i32 m, vec4_i32 v) { return mat4_i32_mul_vec4(m, v); }
#endif

#ifdef GEOM_MAT3_IDENTITY_ALIAS
static inline mat3_i32 GEOM_MAT3_IDENTITY_ALIAS(void) { return mat3_i32_identity(); }
#endif

#ifdef GEOM_MAT4_IDENTITY_ALIAS
static inline mat4_i32 GEOM_MAT4_IDENTITY_ALIAS(void) { return mat4_i32_identity(); }
#endif

#ifdef GEOM_MAT4_TRANSLATE_ALIAS
static inline mat4_i32 GEOM_MAT4_TRANSLATE_ALIAS(i32 x, i32 y, i32 z) { return mat4_i32_translate(x, y, z); }
#endif

#ifdef GEOM_MAT4_SCALE_ALIAS
static inline mat4_i32 GEOM_MAT4_SCALE_ALIAS(i32 x, i32 y, i32 z) { return mat4_i32_scale(x, y, z); }
#endif

// Undefine all alias macros to avoid namespace pollution
#undef GEOM_VEC2_ALIAS
#undef GEOM_VEC3_ALIAS
#undef GEOM_VEC4_ALIAS
#undef GEOM_MAT3_ALIAS
#undef GEOM_MAT4_ALIAS
#undef GEOM_VEC2_ADD_ALIAS
#undef GEOM_VEC3_ADD_ALIAS
#undef GEOM_VEC4_ADD_ALIAS
#undef GEOM_VEC2_SUB_ALIAS
#undef GEOM_VEC3_SUB_ALIAS
#undef GEOM_VEC4_SUB_ALIAS
#undef GEOM_VEC2_MUL_SCALAR_ALIAS
#undef GEOM_VEC3_MUL_SCALAR_ALIAS
#undef GEOM_VEC4_MUL_SCALAR_ALIAS
#undef GEOM_VEC2_DOT_ALIAS
#undef GEOM_VEC3_DOT_ALIAS
#undef GEOM_VEC4_DOT_ALIAS
#undef GEOM_VEC3_CROSS_ALIAS
#undef GEOM_MAT3_MUL_ALIAS
#undef GEOM_MAT4_MUL_ALIAS
#undef GEOM_MAT3_MUL_VEC3_ALIAS
#undef GEOM_MAT4_MUL_VEC4_ALIAS
#undef GEOM_MAT3_IDENTITY_ALIAS
#undef GEOM_MAT4_IDENTITY_ALIAS
#undef GEOM_MAT4_TRANSLATE_ALIAS
#undef GEOM_MAT4_SCALE_ALIAS
