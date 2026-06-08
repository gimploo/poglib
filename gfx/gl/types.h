#pragma once
#include "./objects.h"
#include "./common.h"

typedef union {
    f32 data[8];
    struct {
        vec2f_t t00;
        vec2f_t t10;
        vec2f_t t11;
        vec2f_t t01;
    } ;
} sprite_uv_t;

typedef union {
    f32 data[48];
    struct {
        sprite_uv_t front, back;
        sprite_uv_t left, right;
        sprite_uv_t top, bottom;
    };
} cube_uv_t;

sprite_uv_t sprite_uv(const vec2i_t sprite_count, const u32 index)
{
    /* NOTE:
     * ====
     * 16 X 16 Atlas texture
     * -------------------------
     * 0 | 1 | 2 | 3 | ...   |15
     * -------------------------
     * 16 | 17 | 18 | 19 | ...|30
     * -------------------------
     * . 
     * .
     * .
     */

    const vec2f_t sprite_dim = { 1.0f / sprite_count.x, 1.0f / sprite_count.y };

    vec3i_t row = {0};
    u32 row_max = sprite_count.x;
    while (index >= row_max) {
        row_max *= 2;
        row.y++;
    }
    row.x = index - (sprite_count.x * row.y) ;

    const vec2f_t t00 = {
        row.x * sprite_dim.x, 1.0f - ((row.y + 1.0f) * sprite_dim.y)
    };

    const vec2f_t t10 = {
        (row.x + 1.0f) * (sprite_dim.x) , t00.y
    };

    const vec2f_t t11 = {
        t10.x, 1.0f - (row.y * sprite_dim.y)
    };

    const vec2f_t t01 = {
        t00.x, t11.y
    };

    return (sprite_uv_t) {

        t00.x, t00.y,
        t10.x, t10.y,
        t11.x, t11.y,
        t01.x, t01.y,
    };
}

#define QUAD_UV_DEFAULT quadf(vec3f(0.0f), 1.0f, 1.0f)

typedef struct glvertex2d_t {

    vec3f_t position;
    vec4f_t color;
    vec2f_t uv;

} glvertex2d_t ;

typedef struct glvertex3d_t {

    vec4f_t bone_weights;
    vec4i_t bone_ids;
    vec3f_t bitangents;
    vec3f_t pos;
    vec3f_t norm;
    vec3f_t tangents;
    vec2f_t uv;

} glvertex3d_t ;

void print_glvertex3d(void *data)
{
    glvertex3d_t *v = data;
    printf("POS:        "VEC3F_FMT"\n", VEC3F_ARG(v->pos));
    printf("NORM:       "VEC3F_FMT"\n", VEC3F_ARG(v->norm));
    printf("UV:         "VEC2F_FMT"\n", VEC2F_ARG(v->uv));
    printf("TAN:        "VEC3F_FMT"\n", VEC3F_ARG(v->tangents));
    printf("BITAN:      "VEC3F_FMT"\n", VEC3F_ARG(v->bitangents));
    printf("BONEIDS:    "VEC4I_FMT"\n", VEC4I_ARG(v->bone_ids));
    printf("BONE_WGHT:  "VEC4F_FMT"\n", VEC4F_ARG(v->bone_weights));
}

typedef struct glmesh_t {

    slot_t vtx;
    slot_t idx;
    u8 material_index;

} glmesh_t;

typedef enum glmesh_primitive_type {
    GL_MESH_PRIMITIVE_TYPE_CUBE = 0,
    GL_MESH_PRIMITIVE_TYPE_CAPSULE = 1,
    GL_MESH_PRIMITIVE_TYPE_CAMERA = 2,
    GL_MESH_PRIMITVE_TYPE_COUNT
} glmesh_primitive_type;

void glmesh_destroy(glmesh_t *self)
{
    slot_destroy(&self->vtx);
    slot_destroy(&self->idx);
}

// Capsule: 16 radial segments, 4 hemisphere rings, 2 cylinder subdivisions (178 verts, 1056 indices)
const u32 DEFAULT_CAPSULE_INDICES[] = {
    0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5,
    0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 9,
    0, 9, 10, 0, 10, 11, 0, 11, 12, 0, 12, 13,
    0, 13, 14, 0, 14, 15, 0, 15, 16, 0, 16, 1,
    1, 2, 18, 1, 18, 17, 2, 3, 19, 2, 19, 18,
    3, 4, 20, 3, 20, 19, 4, 5, 21, 4, 21, 20,
    5, 6, 22, 5, 22, 21, 6, 7, 23, 6, 23, 22,
    7, 8, 24, 7, 24, 23, 8, 9, 25, 8, 25, 24,
    9, 10, 26, 9, 26, 25, 10, 11, 27, 10, 27, 26,
    11, 12, 28, 11, 28, 27, 12, 13, 29, 12, 29, 28,
    13, 14, 30, 13, 30, 29, 14, 15, 31, 14, 31, 30,
    15, 16, 32, 15, 32, 31, 16, 1, 17, 16, 17, 32,
    17, 18, 34, 17, 34, 33, 18, 19, 35, 18, 35, 34,
    19, 20, 36, 19, 36, 35, 20, 21, 37, 20, 37, 36,
    21, 22, 38, 21, 38, 37, 22, 23, 39, 22, 39, 38,
    23, 24, 40, 23, 40, 39, 24, 25, 41, 24, 41, 40,
    25, 26, 42, 25, 42, 41, 26, 27, 43, 26, 43, 42,
    27, 28, 44, 27, 44, 43, 28, 29, 45, 28, 45, 44,
    29, 30, 46, 29, 46, 45, 30, 31, 47, 30, 47, 46,
    31, 32, 48, 31, 48, 47, 32, 17, 33, 32, 33, 48,
    33, 34, 50, 33, 50, 49, 34, 35, 51, 34, 51, 50,
    35, 36, 52, 35, 52, 51, 36, 37, 53, 36, 53, 52,
    37, 38, 54, 37, 54, 53, 38, 39, 55, 38, 55, 54,
    39, 40, 56, 39, 56, 55, 40, 41, 57, 40, 57, 56,
    41, 42, 58, 41, 58, 57, 42, 43, 59, 42, 59, 58,
    43, 44, 60, 43, 60, 59, 44, 45, 61, 44, 61, 60,
    45, 46, 62, 45, 62, 61, 46, 47, 63, 46, 63, 62,
    47, 48, 64, 47, 64, 63, 48, 33, 49, 48, 49, 64,
    49, 50, 66, 49, 66, 65, 50, 51, 67, 50, 67, 66,
    51, 52, 68, 51, 68, 67, 52, 53, 69, 52, 69, 68,
    53, 54, 70, 53, 70, 69, 54, 55, 71, 54, 71, 70,
    55, 56, 72, 55, 72, 71, 56, 57, 73, 56, 73, 72,
    57, 58, 74, 57, 74, 73, 58, 59, 75, 58, 75, 74,
    59, 60, 76, 59, 76, 75, 60, 61, 77, 60, 77, 76,
    61, 62, 78, 61, 78, 77, 62, 63, 79, 62, 79, 78,
    63, 64, 80, 63, 80, 79, 64, 49, 65, 64, 65, 80,
    65, 66, 82, 65, 82, 81, 66, 67, 83, 66, 83, 82,
    67, 68, 84, 67, 84, 83, 68, 69, 85, 68, 85, 84,
    69, 70, 86, 69, 86, 85, 70, 71, 87, 70, 87, 86,
    71, 72, 88, 71, 88, 87, 72, 73, 89, 72, 89, 88,
    73, 74, 90, 73, 90, 89, 74, 75, 91, 74, 91, 90,
    75, 76, 92, 75, 92, 91, 76, 77, 93, 76, 93, 92,
    77, 78, 94, 77, 94, 93, 78, 79, 95, 78, 95, 94,
    79, 80, 96, 79, 96, 95, 80, 65, 81, 80, 81, 96,
    81, 82, 98, 81, 98, 97, 82, 83, 99, 82, 99, 98,
    83, 84, 100, 83, 100, 99, 84, 85, 101, 84, 101, 100,
    85, 86, 102, 85, 102, 101, 86, 87, 103, 86, 103, 102,
    87, 88, 104, 87, 104, 103, 88, 89, 105, 88, 105, 104,
    89, 90, 106, 89, 106, 105, 90, 91, 107, 90, 107, 106,
    91, 92, 108, 91, 108, 107, 92, 93, 109, 92, 109, 108,
    93, 94, 110, 93, 110, 109, 94, 95, 111, 94, 111, 110,
    95, 96, 112, 95, 112, 111, 96, 81, 97, 96, 97, 112,
    97, 98, 114, 97, 114, 113, 98, 99, 115, 98, 115, 114,
    99, 100, 116, 99, 116, 115, 100, 101, 117, 100, 117, 116,
    101, 102, 118, 101, 118, 117, 102, 103, 119, 102, 119, 118,
    103, 104, 120, 103, 120, 119, 104, 105, 121, 104, 121, 120,
    105, 106, 122, 105, 122, 121, 106, 107, 123, 106, 123, 122,
    107, 108, 124, 107, 124, 123, 108, 109, 125, 108, 125, 124,
    109, 110, 126, 109, 126, 125, 110, 111, 127, 110, 127, 126,
    111, 112, 128, 111, 128, 127, 112, 97, 113, 112, 113, 128,
    113, 114, 130, 113, 130, 129, 114, 115, 131, 114, 131, 130,
    115, 116, 132, 115, 132, 131, 116, 117, 133, 116, 133, 132,
    117, 118, 134, 117, 134, 133, 118, 119, 135, 118, 135, 134,
    119, 120, 136, 119, 136, 135, 120, 121, 137, 120, 137, 136,
    121, 122, 138, 121, 138, 137, 122, 123, 139, 122, 139, 138,
    123, 124, 140, 123, 140, 139, 124, 125, 141, 124, 141, 140,
    125, 126, 142, 125, 142, 141, 126, 127, 143, 126, 143, 142,
    127, 128, 144, 127, 144, 143, 128, 113, 129, 128, 129, 144,
    129, 130, 146, 129, 146, 145, 130, 131, 147, 130, 147, 146,
    131, 132, 148, 131, 148, 147, 132, 133, 149, 132, 149, 148,
    133, 134, 150, 133, 150, 149, 134, 135, 151, 134, 151, 150,
    135, 136, 152, 135, 152, 151, 136, 137, 153, 136, 153, 152,
    137, 138, 154, 137, 154, 153, 138, 139, 155, 138, 155, 154,
    139, 140, 156, 139, 156, 155, 140, 141, 157, 140, 157, 156,
    141, 142, 158, 141, 158, 157, 142, 143, 159, 142, 159, 158,
    143, 144, 160, 143, 160, 159, 144, 129, 145, 144, 145, 160,
    145, 146, 162, 145, 162, 161, 146, 147, 163, 146, 163, 162,
    147, 148, 164, 147, 164, 163, 148, 149, 165, 148, 165, 164,
    149, 150, 166, 149, 166, 165, 150, 151, 167, 150, 167, 166,
    151, 152, 168, 151, 168, 167, 152, 153, 169, 152, 169, 168,
    153, 154, 170, 153, 170, 169, 154, 155, 171, 154, 171, 170,
    155, 156, 172, 155, 172, 171, 156, 157, 173, 156, 173, 172,
    157, 158, 174, 157, 174, 173, 158, 159, 175, 158, 175, 174,
    159, 160, 176, 159, 176, 175, 160, 145, 161, 160, 161, 176,
    177, 162, 161, 177, 163, 162, 177, 164, 163, 177, 165, 164,
    177, 166, 165, 177, 167, 166, 177, 168, 167, 177, 169, 168,
    177, 170, 169, 177, 171, 170, 177, 172, 171, 177, 173, 172,
    177, 174, 173, 177, 175, 174, 177, 176, 175, 177, 161, 176
};

const f32 DEFAULT_CAPSULE_VERTICES_WITH_NORMALS[] = {
    0.0f, -2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.309017f, -1.951057f, 0.0f, 0.309017f, -0.951057f, 0.0f,
    0.285494f, -1.951057f, 0.118256f, 0.285494f, -0.951057f, 0.118256f,
    0.218508f, -1.951057f, 0.218508f, 0.218508f, -0.951057f, 0.218508f,
    0.118256f, -1.951057f, 0.285494f, 0.118256f, -0.951057f, 0.285494f,
    0.0f, -1.951057f, 0.309017f, 0.0f, -0.951057f, 0.309017f,
    -0.118256f, -1.951057f, 0.285494f, -0.118256f, -0.951057f, 0.285494f,
    -0.218508f, -1.951057f, 0.218508f, -0.218508f, -0.951057f, 0.218508f,
    -0.285494f, -1.951057f, 0.118256f, -0.285494f, -0.951057f, 0.118256f,
    -0.309017f, -1.951057f, 0.0f, -0.309017f, -0.951057f, 0.0f,
    -0.285494f, -1.951057f, -0.118256f, -0.285494f, -0.951057f, -0.118256f,
    -0.218508f, -1.951057f, -0.218508f, -0.218508f, -0.951057f, -0.218508f,
    -0.118256f, -1.951057f, -0.285494f, -0.118256f, -0.951057f, -0.285494f,
    -0.0f, -1.951057f, -0.309017f, -0.0f, -0.951057f, -0.309017f,
    0.118256f, -1.951057f, -0.285494f, 0.118256f, -0.951057f, -0.285494f,
    0.218508f, -1.951057f, -0.218508f, 0.218508f, -0.951057f, -0.218508f,
    0.285494f, -1.951057f, -0.118256f, 0.285494f, -0.951057f, -0.118256f,
    0.587785f, -1.809017f, 0.0f, 0.587785f, -0.809017f, 0.0f,
    0.543043f, -1.809017f, 0.224936f, 0.543043f, -0.809017f, 0.224936f,
    0.415627f, -1.809017f, 0.415627f, 0.415627f, -0.809017f, 0.415627f,
    0.224936f, -1.809017f, 0.543043f, 0.224936f, -0.809017f, 0.543043f,
    0.0f, -1.809017f, 0.587785f, 0.0f, -0.809017f, 0.587785f,
    -0.224936f, -1.809017f, 0.543043f, -0.224936f, -0.809017f, 0.543043f,
    -0.415627f, -1.809017f, 0.415627f, -0.415627f, -0.809017f, 0.415627f,
    -0.543043f, -1.809017f, 0.224936f, -0.543043f, -0.809017f, 0.224936f,
    -0.587785f, -1.809017f, 0.0f, -0.587785f, -0.809017f, 0.0f,
    -0.543043f, -1.809017f, -0.224936f, -0.543043f, -0.809017f, -0.224936f,
    -0.415627f, -1.809017f, -0.415627f, -0.415627f, -0.809017f, -0.415627f,
    -0.224936f, -1.809017f, -0.543043f, -0.224936f, -0.809017f, -0.543043f,
    -0.0f, -1.809017f, -0.587785f, -0.0f, -0.809017f, -0.587785f,
    0.224936f, -1.809017f, -0.543043f, 0.224936f, -0.809017f, -0.543043f,
    0.415627f, -1.809017f, -0.415627f, 0.415627f, -0.809017f, -0.415627f,
    0.543043f, -1.809017f, -0.224936f, 0.543043f, -0.809017f, -0.224936f,
    0.809017f, -1.587785f, 0.0f, 0.809017f, -0.587785f, 0.0f,
    0.747434f, -1.587785f, 0.309597f, 0.747434f, -0.587785f, 0.309597f,
    0.572061f, -1.587785f, 0.572061f, 0.572061f, -0.587785f, 0.572061f,
    0.309597f, -1.587785f, 0.747434f, 0.309597f, -0.587785f, 0.747434f,
    0.0f, -1.587785f, 0.809017f, 0.0f, -0.587785f, 0.809017f,
    -0.309597f, -1.587785f, 0.747434f, -0.309597f, -0.587785f, 0.747434f,
    -0.572061f, -1.587785f, 0.572061f, -0.572061f, -0.587785f, 0.572061f,
    -0.747434f, -1.587785f, 0.309597f, -0.747434f, -0.587785f, 0.309597f,
    -0.809017f, -1.587785f, 0.0f, -0.809017f, -0.587785f, 0.0f,
    -0.747434f, -1.587785f, -0.309597f, -0.747434f, -0.587785f, -0.309597f,
    -0.572061f, -1.587785f, -0.572061f, -0.572061f, -0.587785f, -0.572061f,
    -0.309597f, -1.587785f, -0.747434f, -0.309597f, -0.587785f, -0.747434f,
    -0.0f, -1.587785f, -0.809017f, -0.0f, -0.587785f, -0.809017f,
    0.309597f, -1.587785f, -0.747434f, 0.309597f, -0.587785f, -0.747434f,
    0.572061f, -1.587785f, -0.572061f, 0.572061f, -0.587785f, -0.572061f,
    0.747434f, -1.587785f, -0.309597f, 0.747434f, -0.587785f, -0.309597f,
    0.951057f, -1.309017f, 0.0f, 0.951057f, -0.309017f, 0.0f,
    0.878662f, -1.309017f, 0.363954f, 0.878662f, -0.309017f, 0.363954f,
    0.672499f, -1.309017f, 0.672499f, 0.672499f, -0.309017f, 0.672499f,
    0.363954f, -1.309017f, 0.878662f, 0.363954f, -0.309017f, 0.878662f,
    0.0f, -1.309017f, 0.951057f, 0.0f, -0.309017f, 0.951057f,
    -0.363954f, -1.309017f, 0.878662f, -0.363954f, -0.309017f, 0.878662f,
    -0.672499f, -1.309017f, 0.672499f, -0.672499f, -0.309017f, 0.672499f,
    -0.878662f, -1.309017f, 0.363954f, -0.878662f, -0.309017f, 0.363954f,
    -0.951057f, -1.309017f, 0.0f, -0.951057f, -0.309017f, 0.0f,
    -0.878662f, -1.309017f, -0.363954f, -0.878662f, -0.309017f, -0.363954f,
    -0.672499f, -1.309017f, -0.672499f, -0.672499f, -0.309017f, -0.672499f,
    -0.363954f, -1.309017f, -0.878662f, -0.363954f, -0.309017f, -0.878662f,
    -0.0f, -1.309017f, -0.951057f, -0.0f, -0.309017f, -0.951057f,
    0.363954f, -1.309017f, -0.878662f, 0.363954f, -0.309017f, -0.878662f,
    0.672499f, -1.309017f, -0.672499f, 0.672499f, -0.309017f, -0.672499f,
    0.878662f, -1.309017f, -0.363954f, 0.878662f, -0.309017f, -0.363954f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.92388f, -1.0f, 0.382683f, 0.92388f, 0.0f, 0.382683f,
    0.707107f, -1.0f, 0.707107f, 0.707107f, 0.0f, 0.707107f,
    0.382683f, -1.0f, 0.92388f, 0.382683f, 0.0f, 0.92388f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.382683f, -1.0f, 0.92388f, -0.382683f, 0.0f, 0.92388f,
    -0.707107f, -1.0f, 0.707107f, -0.707107f, 0.0f, 0.707107f,
    -0.92388f, -1.0f, 0.382683f, -0.92388f, 0.0f, 0.382683f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.92388f, -1.0f, -0.382683f, -0.92388f, 0.0f, -0.382683f,
    -0.707107f, -1.0f, -0.707107f, -0.707107f, 0.0f, -0.707107f,
    -0.382683f, -1.0f, -0.92388f, -0.382683f, 0.0f, -0.92388f,
    -0.0f, -1.0f, -1.0f, -0.0f, 0.0f, -1.0f,
    0.382683f, -1.0f, -0.92388f, 0.382683f, 0.0f, -0.92388f,
    0.707107f, -1.0f, -0.707107f, 0.707107f, 0.0f, -0.707107f,
    0.92388f, -1.0f, -0.382683f, 0.92388f, 0.0f, -0.382683f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.92388f, 0.0f, 0.382683f, 0.92388f, 0.0f, 0.382683f,
    0.707107f, 0.0f, 0.707107f, 0.707107f, 0.0f, 0.707107f,
    0.382683f, 0.0f, 0.92388f, 0.382683f, 0.0f, 0.92388f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.382683f, 0.0f, 0.92388f, -0.382683f, 0.0f, 0.92388f,
    -0.707107f, 0.0f, 0.707107f, -0.707107f, 0.0f, 0.707107f,
    -0.92388f, 0.0f, 0.382683f, -0.92388f, 0.0f, 0.382683f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.92388f, 0.0f, -0.382683f, -0.92388f, 0.0f, -0.382683f,
    -0.707107f, 0.0f, -0.707107f, -0.707107f, 0.0f, -0.707107f,
    -0.382683f, 0.0f, -0.92388f, -0.382683f, 0.0f, -0.92388f,
    -0.0f, 0.0f, -1.0f, -0.0f, 0.0f, -1.0f, 0.382683f, 0.0f, -0.92388f, 0.382683f, 0.0f, -0.92388f,
    0.707107f, 0.0f, -0.707107f, 0.707107f, 0.0f, -0.707107f,
    0.92388f, 0.0f, -0.382683f, 0.92388f, 0.0f, -0.382683f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.92388f, 1.0f, 0.382683f, 0.92388f, 0.0f, 0.382683f,
    0.707107f, 1.0f, 0.707107f, 0.707107f, 0.0f, 0.707107f,
    0.382683f, 1.0f, 0.92388f, 0.382683f, 0.0f, 0.92388f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.382683f, 1.0f, 0.92388f, -0.382683f, 0.0f, 0.92388f,
    -0.707107f, 1.0f, 0.707107f, -0.707107f, 0.0f, 0.707107f,
    -0.92388f, 1.0f, 0.382683f, -0.92388f, 0.0f, 0.382683f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.92388f, 1.0f, -0.382683f, -0.92388f, 0.0f, -0.382683f,
    -0.707107f, 1.0f, -0.707107f, -0.707107f, 0.0f, -0.707107f,
    -0.382683f, 1.0f, -0.92388f, -0.382683f, 0.0f, -0.92388f,
    -0.0f, 1.0f, -1.0f, -0.0f, 0.0f, -1.0f, 0.382683f, 1.0f, -0.92388f, 0.382683f, 0.0f, -0.92388f,
    0.707107f, 1.0f, -0.707107f, 0.707107f, 0.0f, -0.707107f,
    0.92388f, 1.0f, -0.382683f, 0.92388f, 0.0f, -0.382683f,
    0.309017f, 1.951057f, 0.0f, 0.309017f, 0.951057f, 0.0f,
    0.285494f, 1.951057f, 0.118256f, 0.285494f, 0.951057f, 0.118256f,
    0.218508f, 1.951057f, 0.218508f, 0.218508f, 0.951057f, 0.218508f,
    0.118256f, 1.951057f, 0.285494f, 0.118256f, 0.951057f, 0.285494f,
    0.0f, 1.951057f, 0.309017f, 0.0f, 0.951057f, 0.309017f,
    -0.118256f, 1.951057f, 0.285494f, -0.118256f, 0.951057f, 0.285494f,
    -0.218508f, 1.951057f, 0.218508f, -0.218508f, 0.951057f, 0.218508f,
    -0.285494f, 1.951057f, 0.118256f, -0.285494f, 0.951057f, 0.118256f,
    -0.309017f, 1.951057f, 0.0f, -0.309017f, 0.951057f, 0.0f,
    -0.285494f, 1.951057f, -0.118256f, -0.285494f, 0.951057f, -0.118256f,
    -0.218508f, 1.951057f, -0.218508f, -0.218508f, 0.951057f, -0.218508f,
    -0.118256f, 1.951057f, -0.285494f, -0.118256f, 0.951057f, -0.285494f,
    -0.0f, 1.951057f, -0.309017f, -0.0f, 0.951057f, -0.309017f,
    0.118256f, 1.951057f, -0.285494f, 0.118256f, 0.951057f, -0.285494f,
    0.218508f, 1.951057f, -0.218508f, 0.218508f, 0.951057f, -0.218508f,
    0.285494f, 1.951057f, -0.118256f, 0.285494f, 0.951057f, -0.118256f,
    0.587785f, 1.809017f, 0.0f, 0.587785f, 0.809017f, 0.0f,
    0.543043f, 1.809017f, 0.224936f, 0.543043f, 0.809017f, 0.224936f,
    0.415627f, 1.809017f, 0.415627f, 0.415627f, 0.809017f, 0.415627f,
    0.224936f, 1.809017f, 0.543043f, 0.224936f, 0.809017f, 0.543043f,
    0.0f, 1.809017f, 0.587785f, 0.0f, 0.809017f, 0.587785f,
    -0.224936f, 1.809017f, 0.543043f, -0.224936f, 0.809017f, 0.543043f,
    -0.415627f, 1.809017f, 0.415627f, -0.415627f, 0.809017f, 0.415627f,
    -0.543043f, 1.809017f, 0.224936f, -0.543043f, 0.809017f, 0.224936f,
    -0.587785f, 1.809017f, 0.0f, -0.587785f, 0.809017f, 0.0f,
    -0.543043f, 1.809017f, -0.224936f, -0.543043f, 0.809017f, -0.224936f,
    -0.415627f, 1.809017f, -0.415627f, -0.415627f, 0.809017f, -0.415627f,
    -0.224936f, 1.809017f, -0.543043f, -0.224936f, 0.809017f, -0.543043f,
    -0.0f, 1.809017f, -0.587785f, -0.0f, 0.809017f, -0.587785f,
    0.224936f, 1.809017f, -0.543043f, 0.224936f, 0.809017f, -0.543043f,
    0.415627f, 1.809017f, -0.415627f, 0.415627f, 0.809017f, -0.415627f,
    0.543043f, 1.809017f, -0.224936f, 0.543043f, 0.809017f, -0.224936f,
    0.809017f, 1.587785f, 0.0f, 0.809017f, 0.587785f, 0.0f,
    0.747434f, 1.587785f, 0.309597f, 0.747434f, 0.587785f, 0.309597f,
    0.572061f, 1.587785f, 0.572061f, 0.572061f, 0.587785f, 0.572061f,
    0.309597f, 1.587785f, 0.747434f, 0.309597f, 0.587785f, 0.747434f,
    0.0f, 1.587785f, 0.809017f, 0.0f, 0.587785f, 0.809017f,
    -0.309597f, 1.587785f, 0.747434f, -0.309597f, 0.587785f, 0.747434f,
    -0.572061f, 1.587785f, 0.572061f, -0.572061f, 0.587785f, 0.572061f,
    -0.747434f, 1.587785f, 0.309597f, -0.747434f, 0.587785f, 0.309597f,
    -0.809017f, 1.587785f, 0.0f, -0.809017f, 0.587785f, 0.0f,
    -0.747434f, 1.587785f, -0.309597f, -0.747434f, 0.587785f, -0.309597f,
    -0.572061f, 1.587785f, -0.572061f, -0.572061f, 0.587785f, -0.572061f,
    -0.309597f, 1.587785f, -0.747434f, -0.309597f, 0.587785f, -0.747434f,
    -0.0f, 1.587785f, -0.809017f, -0.0f, 0.587785f, -0.809017f,
    0.309597f, 1.587785f, -0.747434f, 0.309597f, 0.587785f, -0.747434f,
    0.572061f, 1.587785f, -0.572061f, 0.572061f, 0.587785f, -0.572061f,
    0.747434f, 1.587785f, -0.309597f, 0.747434f, 0.587785f, -0.309597f,
    0.951057f, 1.309017f, 0.0f, 0.951057f, 0.309017f, 0.0f,
    0.878662f, 1.309017f, 0.363954f, 0.878662f, 0.309017f, 0.363954f,
    0.672499f, 1.309017f, 0.672499f, 0.672499f, 0.309017f, 0.672499f,
    0.363954f, 1.309017f, 0.878662f, 0.363954f, 0.309017f, 0.878662f,
    0.0f, 1.309017f, 0.951057f, 0.0f, 0.309017f, 0.951057f,
    -0.363954f, 1.309017f, 0.878662f, -0.363954f, 0.309017f, 0.878662f,
    -0.672499f, 1.309017f, 0.672499f, -0.672499f, 0.309017f, 0.672499f,
    -0.878662f, 1.309017f, 0.363954f, -0.878662f, 0.309017f, 0.363954f,
    -0.951057f, 1.309017f, 0.0f, -0.951057f, 0.309017f, 0.0f,
    -0.878662f, 1.309017f, -0.363954f, -0.878662f, 0.309017f, -0.363954f,
    -0.672499f, 1.309017f, -0.672499f, -0.672499f, 0.309017f, -0.672499f,
    -0.363954f, 1.309017f, -0.878662f, -0.363954f, 0.309017f, -0.878662f,
    -0.0f, 1.309017f, -0.951057f, -0.0f, 0.309017f, -0.951057f,
    0.363954f, 1.309017f, -0.878662f, 0.363954f, 0.309017f, -0.878662f,
    0.672499f, 1.309017f, -0.672499f, 0.672499f, 0.309017f, -0.672499f,
    0.878662f, 1.309017f, -0.363954f, 0.878662f, 0.309017f, -0.363954f,
    0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f
};

const f32 DEFAULT_CUBE_VERTICES_8[] = {
     // front
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    // back
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
};

typedef enum {

    RIGHT  = 0,
    LEFT   = 1,
    TOP    = 2,
    BOTTOM = 3,
    FRONT   = 4,
    BACK  = 5,
    TOTAL_CUBE_FACES = 6

} CUBE_FACES ;

const f32 DEFAULT_CUBE_VERTICES_WITH_NORMALS_AND_UVS_24[] = {
    // Pos [0,1,2] | Norm [3,4,5] | UV [6,7]

    // front face (+Z)
    -1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // BL
     1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // BR
     1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // TR
    -1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // TL

    // back face (-Z)
     1.0f, -1.0f, -1.0f,   0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,   0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,   0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f, -1.0f,   0.0f, 0.0f, -1.0f,  0.0f, 1.0f,

    // left face (-X)
    -1.0f, -1.0f, -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,

    // right face (+X)
     1.0f, -1.0f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,

    // top face (+Y)
    -1.0f,  1.0f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
     1.0f,  1.0f,  1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

    // bottom face (-Y)
    -1.0f, -1.0f, -1.0f,   0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,   0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
     1.0f, -1.0f,  1.0f,   0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,   0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
};

// NOTE: for 3d textured cubes
const f32 DEFAULT_CUBE_VERTICES_24[] = {
     // front
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,

    // back
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    //left
    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,

    //right
    1.0f, -1.0f, 1.0f,
    +1.0f, -1.0f, -1.0f, 
    +1.0f, 1.0f, -1.0f,
    +1.0f, 1.0f, 1.0f,

    //top
    1.0f, 1.0f, 1.0f, 
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f, 
    -1.0f, 1.0f, 1.0f,

    //bottom
    1.0f, -1.0f, 1.0f, 
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, 
    -1.0f, -1.0f, 1.0f,
};

const u32 DEFAULT_CUBE_INDICES_24[] = {
    // Front (+Z)
    0, 1, 2,    2, 3, 0,
    // Back (-Z)
    4, 5, 6,    6, 7, 4,
    // Left (-X)
    8, 9, 10,   10, 11, 8,
    // Right (+X)
    12, 13, 14, 14, 15, 12,
    // Top (+Y)
    16, 17, 18, 18, 19, 16,
    // Bottom (-Y)
    20, 21, 22, 22, 23, 20 
};

const u32 DEFAULT_CUBE_INDICES_8[] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // back
    4, 5, 6,
    6, 7, 4,
    // right
    1, 5, 6,
    6, 2, 1,
    // left
    0, 4, 7,
    7, 3, 0,
    // bottom
    4, 5, 1,
    1, 0, 4,
    // top
    7, 6, 2,
    2, 3, 7
};

typedef struct { glvertex2d_t vertex[3]; } gltri_t;
typedef struct { glvertex2d_t vertex[4]; } glquad_t;
typedef struct { glvertex2d_t vertex[MAX_VERTICES_PER_CIRCLE]; } glcircle_t;

typedef struct {

    glcircle_t  vertices;
    u8          sides;

} glpolygon_t ;

#define MAX_VERTICES_PER_CUBE   72
#define MAX_UVS_PER_CUBE        36


gltri_t         gltri(trif_t tri, vec4f_t color, quadf_t tex_coord);
glquad_t        glquad(const quadf_t positions, const vec4f_t color, const rect_t tex_coord);
glcircle_t      glcircle(circle_t circle, vec4f_t color, quadf_t uv);
glpolygon_t     glpolygon(polygon_t polygon, vec4f_t color, quadf_t uv);

typedef enum {

    GLBT_gltri_t = 0,
    GLBT_glquad_t,
    GLBT_glcircle_t,
    GLBT_glpolygon_t,
    GLBT_COUNT

} glbatch_type;

typedef struct glbatch_t {

    queue_t globjs;

    struct {
        const glbatch_type  type;
        i8                  nvertex;
    } __meta;

    vao_t vao;
    vbo_t vbo;
    ebo_t ebo;

} glbatch_t ;

#define         glbatch_init(CAPACITY, TYPE)    __impl_glbatch_init((CAPACITY), GLBT_type(TYPE), #TYPE)
#define         glbatch_put(PBATCH, ELEM)       __impl_glbatch_put((PBATCH), &(ELEM), sizeof(ELEM))
#define         glbatch_get(PBATCH, ELEM)       queue_get_in_buffer(&(PBATCH)->globjs, (ELEM))
#define         glbatch_is_empty(PBATCH)        queue_is_empty(&(PBATCH)->globjs)
void            glbatch_combine(glbatch_t *dest, glbatch_t *src);
#define         glbatch_clear(PBATCH)           queue_clear(&(PBATCH)->globjs)
void            glbatch_destroy(glbatch_t *batch);


typedef struct {

    glbatch_t data;

} gltext_t;

gltext_t        gltext_init(const u64 capacity);
#define         gltext_put(PTEXT, ELEM)         queue_put(&(PTEXT)->data.globjs, (ELEM))
#define         gltext_get(PTEXT, ELEM)         queue_get_in_buffer(&(PTEXT)->data.globjs, (ELEM))
#define         gltext_is_empty(PTEXT)          queue_is_empty(&(PTEXT)->data.globjs)
#define         gltext_clear(PTEXT)             queue_clear(&(PTEXT)->data.globjs)
#define         gltext_destroy(PTEXT)           glbatch_destroy(&(PTEXT)->data)


#ifndef IGNORE_GL_TYPE_IMPLEMENTATION

#define GLBT_type(TYPE) GLBT_##TYPE
#define MAX_TRI_INDICES_CAPACITY 3
#define MAX_QUAD_INDICES_CAPACITY 6

void __impl_glbatch_put(glbatch_t *batch, const void *elem, const u64 elemsize)
{
    switch(batch->__meta.type)
    {
        case GLBT_gltri_t:
        case GLBT_glquad_t:
        case GLBT_glcircle_t:
            __impl_queue_put(&batch->globjs, elem, elemsize);
        break;

        case GLBT_glpolygon_t: {

            glpolygon_t *poly = (glpolygon_t *)elem;

            if (batch->__meta.nvertex == -1) {

                batch->__meta.nvertex = poly->sides * 3;

                queue_t oldqueue = batch->globjs;
                queue_destroy(&batch->globjs);
                eprint("Refactor: do we even need this anymore");
                /*
                batch->globjs = __impl_queue_init(
                        oldqueue.__capacity,
                        poly->sides * 3 * sizeof(glvertex2d_t ),
                        "glpolygon_t");
                */
            }

            if ((poly->sides * 3) != batch->__meta.nvertex)
               eprint("Trying to push a polygon of `%i` vertices to a batch expecting polygon of `%u` vertices", 
                       poly->sides,
                       batch->__meta.nvertex);

            const u64 size = 
                sizeof(glvertex2d_t ) * batch->__meta.nvertex;

            __impl_queue_put(
                    &batch->globjs, 
                    &poly->vertices, 
                    size);
        } break;

        default: eprint("batch type not accounted for");
    }

}



const u32 DEFAULT_TRI_INDICES[] = {
    0, 1, 2
};

const vec2f_t DEFAULT_QUAD_VTX[4] = {
    -1.0f, -1.0f, // Bottom Left
     1.0f, -1.0f, // Bottom Right
     1.0f,  1.0f, // Top Right
    -1.0f,  1.0f, // Top Left
};

const vec2f_t DEFAULT_QUAD__4TH_QUADRANT_VTX[4] = {
    {0.0f, 0.0f}, // Top Left
    {1.0f, 0.0f}, // Top Right
    {1.0f, 1.0f}, // Bottom Right
    {0.0f, 1.0f}, // Bottom Left
};

const u32 DEFAULT_QUAD_INDICES[] = {
    0, 1, 2,
    2, 3, 0
};

#define GENERATE_QUAD_IDX(OFFSET)\
    {\
        [0] = 4 * OFFSET + DEFAULT_QUAD_INDICES[0],\
        [1] = 4 * OFFSET + DEFAULT_QUAD_INDICES[1],\
        [2] = 4 * OFFSET + DEFAULT_QUAD_INDICES[2],\
        [3] = 4 * OFFSET + DEFAULT_QUAD_INDICES[3],\
        [4] = 4 * OFFSET + DEFAULT_QUAD_INDICES[4],\
        [5] = 4 * OFFSET + DEFAULT_QUAD_INDICES[5],\
    }

// Creates a quad suited for OpenGL
glquad_t glquad(const quadf_t positions, const vec4f_t color, const rect_t tex_coord)
{
    return (glquad_t) { 

        .vertex = {
            [TOP_LEFT] = (glvertex2d_t ){ 
                positions.vertex[0].raw[X], positions.vertex[0].raw[Y], positions.vertex[0].raw[Z], 
                color, 
                tex_coord.vertex[0].raw[X], tex_coord.vertex[0].raw[Y],
            },
            [TOP_RIGHT] = (glvertex2d_t ){ 
                positions.vertex[1].raw[X], positions.vertex[1].raw[Y], positions.vertex[1].raw[Z], 
                color, 
                tex_coord.vertex[1].raw[X], tex_coord.vertex[1].raw[Y],
            }, 
            [BOTTOM_RIGHT] = (glvertex2d_t ){ 
                positions.vertex[2].raw[X], positions.vertex[2].raw[Y], positions.vertex[2].raw[Z], 
                color, 
                tex_coord.vertex[2].raw[X], tex_coord.vertex[2].raw[Y],
            }, 
            [BOTTOM_LEFT] = (glvertex2d_t ){ 
                positions.vertex[3].raw[X], positions.vertex[3].raw[Y], positions.vertex[3].raw[Z], 
                color, 
                tex_coord.vertex[3].raw[X], tex_coord.vertex[3].raw[Y],
            } 
        }
    };
}

gltri_t gltri(trif_t tri, vec4f_t color, quadf_t tex_coord)
{
    return (gltri_t) {

        .vertex[0] = (glvertex2d_t ){ 
            tri.vertex[0].raw[X], tri.vertex[0].raw[Y], tri.vertex[0].raw[Z], 
            color, 
            tex_coord.vertex[0].raw[X], tex_coord.vertex[0].raw[Y],
        }, 
        .vertex[1] = (glvertex2d_t ){ 
            tri.vertex[1].raw[X], tri.vertex[1].raw[Y], tri.vertex[1].raw[Z], 
            color, 
            tex_coord.vertex[1].raw[X], tex_coord.vertex[1].raw[Y],
        }, 
        .vertex[2] = (glvertex2d_t ) { 
            tri.vertex[2].raw[X], tri.vertex[2].raw[Y], tri.vertex[2].raw[Z], 
            color, 
            tex_coord.vertex[2].raw[X], tex_coord.vertex[2].raw[Y],
        }, 
    };
}

glpolygon_t glpolygon(polygon_t polygon, vec4f_t color, quadf_t uv)
{
    glpolygon_t output = {0} ;

    glvertex2d_t *vertices = output.vertices.vertex;

    // TODO: Textures on polygons
    //for (u64 i = 0; i < MAX_TRIANGLES_PER_CIRCLE; i++)
    //{
        //uv.vertex[i].raw[X] = (polygon.points[i].raw[X] /polygon.radius + 1)*0.5;
        //uv.vertex[i].raw[Y] = (polygon.points[i].raw[Y]/polygon.radius + 1)*0.5;

         //float tx = (x/r + 1)*0.5;
         //float ty = (y/r + 1)*0.5;

    //}

    output.sides = polygon.sides;
    for (u64 i = 0; i < (polygon.sides * 3); i++)
    {
        vertices[i].position = polygon.vertices.points[i];
        vertices[i].color = color; 
        vertices[i].uv = glms_vec2(uv.vertex[i]);
    }
    return output;
}

glcircle_t glcircle(circle_t circle, vec4f_t color, quadf_t uv)
{
    glcircle_t output = {0} ;

    glvertex2d_t *vertices = output.vertex;

    // TODO: Textures on circles
    //for (u64 i = 0; i < MAX_TRIANGLES_PER_CIRCLE; i++)
    //{
        //uv.vertex[i].raw[X] = (circle.points[i].raw[X] /circle.radius + 1)*0.5;
        //uv.vertex[i].raw[Y] = (circle.points[i].raw[Y]/circle.radius + 1)*0.5;

         //float tx = (x/r + 1)*0.5;
         //float ty = (y/r + 1)*0.5;

    //}

    for (u64 i = 0; i < MAX_VERTICES_PER_CIRCLE; i++)
    {
        vertices[i].position = circle.points[i];
        vertices[i].color = color; 
        vertices[i].uv = glms_vec2(uv.vertex[i]);
    }
    return output;
}





void __gen_quad_indices(u32 indices[], const u32 shape_count)
{
    memcpy(indices, DEFAULT_QUAD_INDICES, sizeof(DEFAULT_QUAD_INDICES));
    for (u64 i = 1; i < shape_count; i++)
    {
        indices[(i*6) + 0]   = DEFAULT_QUAD_INDICES[0] + (4 * i); 
        indices[(i*6) + 1]   = DEFAULT_QUAD_INDICES[1] + (4 * i);
        indices[(i*6) + 2]   = DEFAULT_QUAD_INDICES[2] + (4 * i);
        indices[(i*6) + 3]   = DEFAULT_QUAD_INDICES[3] + (4 * i);
        indices[(i*6) + 4]   = DEFAULT_QUAD_INDICES[4] + (4 * i);
        indices[(i*6) + 5]   = DEFAULT_QUAD_INDICES[5] + (4 * i);
    }

}

//void __gen_tri_indices(u32 indices[], const u32 shape_count)
//{
    //memcpy(indices, DEFAULT_TRI_INDICES, sizeof(DEFAULT_TRI_INDICES));
    //for (u64 i = 1; i < shape_count; i++)
    //{
        //indices[(i*3) + 0]   = DEFAULT_TRI_INDICES[0] + (3 * i); 
        //indices[(i*3) + 1]   = DEFAULT_TRI_INDICES[1] + (3 * i);
        //indices[(i*3) + 2]   = DEFAULT_TRI_INDICES[2] + (3 * i);
    //}

//}
//
gltext_t gltext_init(const u64 capacity)
{
    return (gltext_t ) {
        .data =  {
            .globjs = __impl_queue_init(capacity, sizeof(glquad_t ), "glquad_t", NULL),
            .__meta = {
                .type       = GLBT_type(glquad_t ),
                .nvertex    = 6 
            }
        }
    };
}

glbatch_t __impl_glbatch_init(u64 capacity, glbatch_type type, const char *type_name)
{
    i8 nvertex = 0;
    u64 typesize = 0;
    switch(type)
    {
        case GLBT_gltri_t:
            nvertex =  3;
            typesize = sizeof(gltri_t );
        break;

        case GLBT_glquad_t:
            nvertex =  6;
            typesize = sizeof(glquad_t);
        break;

        case GLBT_glcircle_t:
            nvertex =  MAX_VERTICES_PER_CIRCLE;
            typesize = sizeof(glcircle_t );
        break;

        case GLBT_glpolygon_t:
            nvertex   =  -1;
            typesize    = 0;
        break;

        default: eprint("batch type not accounted for");
    }
    glbatch_t o =  {
        .globjs   = __impl_queue_init(capacity, typesize, type_name, NULL),
        .__meta = {
            .type = type,
            .nvertex = nvertex
        }
    };

    return o;
}

void glbatch_combine(glbatch_t *dest, glbatch_t *src)
{
    assert(dest->__meta.type == src->__meta.type);

    queue_t *queue = &src->globjs;

    queue_iterator(queue, iter)
    {
        switch(dest->__meta.type)
        {
            case GLBT_glquad_t: {
                glquad_t *quad = (glquad_t *)iter;
                glbatch_put(dest, *quad);
            } break;

            case GLBT_gltri_t: {
                gltri_t *tri = (gltri_t *)iter;
                glbatch_put(dest, *tri);
            } break;

            case GLBT_glcircle_t: {
                glcircle_t *circle = (glcircle_t *)iter;
                glbatch_put(dest, *circle);
            } break;

            default: eprint("type not accounted for");
        }
    }
}


void glbatch_destroy(glbatch_t *batch) 
{
    queue_destroy(&batch->globjs);
}

#endif
