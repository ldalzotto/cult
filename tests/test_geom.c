// Tests for geometry module with aliases
#include "test_framework.h"
#include "geom/primitive.h"
#include "print.h"
#include "file.h"

// Define custom aliases for this test
#define GEOM_VEC3_ALIAS my_vec3
#define GEOM_VEC3_ADD_ALIAS my_vec3_add
#define GEOM_VEC3_DOT_ALIAS my_vec3_dot
#define GEOM_VEC3_CROSS_ALIAS my_vec3_cross
#define GEOM_MAT4_ALIAS my_mat4
#define GEOM_MAT4_IDENTITY_ALIAS my_mat4_identity
#define GEOM_MAT4_TRANSLATE_ALIAS my_mat4_translate
#define GEOM_MAT4_SCALE_ALIAS my_mat4_scale
#define GEOM_MAT4_MUL_ALIAS my_mat4_mul

#include "geom/alias.h"

static void test_geom_aliases_types(test_context* t) {
    // Test that type aliases work
    my_vec3 v1 = {1, 2, 3};
    my_vec3 v2 = {4, 5, 6};

    TEST_ASSERT_EQUAL(t, v1.x, 1);
    TEST_ASSERT_EQUAL(t, v1.y, 2);
    TEST_ASSERT_EQUAL(t, v1.z, 3);

    TEST_ASSERT_EQUAL(t, v2.x, 4);
    TEST_ASSERT_EQUAL(t, v2.y, 5);
    TEST_ASSERT_EQUAL(t, v2.z, 6);
}

static void test_geom_aliases_functions(test_context* t) {
    // Test that function aliases work
    my_vec3 v1 = {1, 2, 3};
    my_vec3 v2 = {4, 5, 6};

    // Test addition
    my_vec3 result_add = my_vec3_add(v1, v2);
    TEST_ASSERT_EQUAL(t, result_add.x, 5);
    TEST_ASSERT_EQUAL(t, result_add.y, 7);
    TEST_ASSERT_EQUAL(t, result_add.z, 9);

    // Test dot product
    i64 dot = my_vec3_dot(v1, v2);
    TEST_ASSERT_EQUAL(t, dot, 1*4 + 2*5 + 3*6); // 4 + 10 + 18 = 32

    // Test cross product
    my_vec3 cross = my_vec3_cross(v1, v2);
    TEST_ASSERT_EQUAL(t, cross.x, 2*6 - 3*5); // 12 - 15 = -3
    TEST_ASSERT_EQUAL(t, cross.y, 3*4 - 1*6); // 12 - 6 = 6
    TEST_ASSERT_EQUAL(t, cross.z, 1*5 - 2*4); // 5 - 8 = -3
}

static void test_geom_aliases_matrices(test_context* t) {
    // Test matrix aliases
    my_mat4 identity = my_mat4_identity();
    TEST_ASSERT_EQUAL(t, identity.m[0], 1);
    TEST_ASSERT_EQUAL(t, identity.m[5], 1);
    TEST_ASSERT_EQUAL(t, identity.m[10], 1);
    TEST_ASSERT_EQUAL(t, identity.m[15], 1);

    my_mat4 translate = my_mat4_translate(10, 20, 30);
    TEST_ASSERT_EQUAL(t, translate.m[3], 10);
    TEST_ASSERT_EQUAL(t, translate.m[7], 20);
    TEST_ASSERT_EQUAL(t, translate.m[11], 30);

    my_mat4 scale = my_mat4_scale(2, 3, 4);
    TEST_ASSERT_EQUAL(t, scale.m[0], 2);
    TEST_ASSERT_EQUAL(t, scale.m[5], 3);
    TEST_ASSERT_EQUAL(t, scale.m[10], 4);

    // Test matrix multiplication
    my_mat4 combined = my_mat4_mul(translate, scale);
    // The result should have scaling on diagonal and translation
    TEST_ASSERT_EQUAL(t, combined.m[0], 2);
    TEST_ASSERT_EQUAL(t, combined.m[5], 3);
    TEST_ASSERT_EQUAL(t, combined.m[10], 4);
    TEST_ASSERT_EQUAL(t, combined.m[3], 10);
    TEST_ASSERT_EQUAL(t, combined.m[7], 20);
    TEST_ASSERT_EQUAL(t, combined.m[11], 30);
}

void test_geom_module(test_context* t) {
    print_string(file_stdout(), STRING("Registering Geometry Module Tests...\n"));
    REGISTER_TEST(t, "geom_aliases_types", test_geom_aliases_types);
    REGISTER_TEST(t, "geom_aliases_functions", test_geom_aliases_functions);
    REGISTER_TEST(t, "geom_aliases_matrices", test_geom_aliases_matrices);
}
