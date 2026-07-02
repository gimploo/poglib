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

//FIXME: this is a wierd naming convention, this having glmesh prefix when its not used within glmesh_t,
//the only place this used so far is in `assetmanager` which i think the the type should denote. 
typedef enum glmesh_primitive_type {
    GL_MESH_PRIMITIVE_TYPE_NONE     = 0,
    GL_MESH_PRIMITIVE_TYPE_LINE     = 1,
    GL_MESH_PRIMITIVE_TYPE_CUBE     = 2,
    GL_MESH_PRIMITIVE_TYPE_CAPSULE  = 3,
    GL_MESH_PRIMITIVE_TYPE_CAMERA   = 4,
    GL_MESH_PRIMITIVE_TYPE_CYLINDER = 5,
    GL_MESH_PRIMITIVE_TYPE_COUNT
} glmesh_primitive_type;

void glmesh_destroy(glmesh_t *self)
{
    slot_destroy(&self->vtx);
    slot_destroy(&self->idx);
}

// Wireframe capsule: 8 radial segments, 1 hemisphere ring (34 verts, 192 indices)
const u32 DEFAULT_CAPSULE_INDICES[] = {
    0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5,
    0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 1,
    1, 10, 9, 1, 2, 10, 2, 11, 10, 2, 3, 11,
    3, 12, 11, 3, 4, 12, 4, 13, 12, 4, 5, 13,
    5, 14, 13, 5, 6, 14, 6, 15, 14, 6, 7, 15,
    7, 16, 15, 7, 8, 16, 8, 9, 16, 8, 1, 9,
    9, 18, 17, 9, 10, 18, 10, 19, 18, 10, 11, 19,
    11, 20, 19, 11, 12, 20, 12, 21, 20, 12, 13, 21,
    13, 22, 21, 13, 14, 22, 14, 23, 22, 14, 15, 23,
    15, 24, 23, 15, 16, 24, 16, 17, 24, 16, 9, 17,
    17, 26, 25, 17, 18, 26, 18, 27, 26, 18, 19, 27,
    19, 28, 27, 19, 20, 28, 20, 29, 28, 20, 21, 29,
    21, 30, 29, 21, 22, 30, 22, 31, 30, 22, 23, 31,
    23, 32, 31, 23, 24, 32, 24, 25, 32, 24, 17, 25,
    33, 26, 25, 33, 27, 26, 33, 28, 27, 33, 29, 28,
    33, 30, 29, 33, 31, 30, 33, 32, 31, 33, 25, 32,
};

const f32 DEFAULT_CAPSULE_VERTICES_WITH_NORMALS[] = {
    0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.707107f, 1.707107f, 0.0f, 0.707107f, 0.707107f, 0.0f,
    0.5f, 1.707107f, 0.5f, 0.5f, 0.707107f, 0.5f,
    0.0f, 1.707107f, 0.707107f, 0.0f, 0.707107f, 0.707107f,
    -0.5f, 1.707107f, 0.5f, -0.5f, 0.707107f, 0.5f,
    -0.707107f, 1.707107f, 0.0f, -0.707107f, 0.707107f, 0.0f,
    -0.5f, 1.707107f, -0.5f, -0.5f, 0.707107f, -0.5f,
    0.0f, 1.707107f, -0.707107f, 0.0f, 0.707107f, -0.707107f,
    0.5f, 1.707107f, -0.5f, 0.5f, 0.707107f, -0.5f,
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.707107f, 1.0f, 0.707107f, 0.707107f, 0.0f, 0.707107f,
    0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.707107f, 1.0f, 0.707107f, -0.707107f, 0.0f, 0.707107f,
    -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.707107f, 1.0f, -0.707107f, -0.707107f, 0.0f, -0.707107f,
    0.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
    0.707107f, 1.0f, -0.707107f, 0.707107f, 0.0f, -0.707107f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.707107f, -1.0f, 0.707107f, 0.707107f, 0.0f, 0.707107f,
    0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.707107f, -1.0f, 0.707107f, -0.707107f, 0.0f, 0.707107f,
    -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.707107f, -1.0f, -0.707107f, -0.707107f, 0.0f, -0.707107f,
    0.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
    0.707107f, -1.0f, -0.707107f, 0.707107f, 0.0f, -0.707107f,
    0.707107f, -1.707107f, 0.0f, 0.707107f, -0.707107f, 0.0f,
    0.5f, -1.707107f, 0.5f, 0.5f, -0.707107f, 0.5f,
    0.0f, -1.707107f, 0.707107f, 0.0f, -0.707107f, 0.707107f,
    -0.5f, -1.707107f, 0.5f, -0.5f, -0.707107f, 0.5f,
    -0.707107f, -1.707107f, 0.0f, -0.707107f, -0.707107f, 0.0f,
    -0.5f, -1.707107f, -0.5f, -0.5f, -0.707107f, -0.5f,
    0.0f, -1.707107f, -0.707107f, 0.0f, -0.707107f, -0.707107f,
    0.5f, -1.707107f, -0.5f, 0.5f, -0.707107f, -0.5f,
    0.0f, -2.0f, 0.0f, 0.0f, -1.0f, 0.0f,
};

const u32 DEFAULT_CYLINDER_INDICES[] = {
    0,1,2, 0,2,3, 0,3,4, 0,4,5,
    0,5,6, 0,6,7, 0,7,8, 0,8,1,
    9,11,10, 9,12,11, 9,13,12, 9,14,13,
    9,15,14, 9,16,15, 9,17,16, 9,10,17,
    18,26,27, 27,19,18,
    19,27,28, 28,20,19,
    20,28,29, 29,21,20,
    21,29,30, 30,22,21,
    22,30,31, 31,23,22,
    23,31,32, 32,24,23,
    24,32,33, 33,25,24,
    25,33,26, 26,18,25,
};

const f32 DEFAULT_CYLINDER_VERTICES_WITH_NORMALS_AND_UVS[] = {
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f,
    1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.5f,
    0.707107f, 1.0f, 0.707107f, 0.0f, 1.0f, 0.0f, 0.853553f, 0.853553f,
    0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f,
    -0.707107f, 1.0f, 0.707107f, 0.0f, 1.0f, 0.0f, 0.146447f, 0.853553f,
    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f,
    -0.707107f, 1.0f, -0.707107f, 0.0f, 1.0f, 0.0f, 0.146447f, 0.146447f,
    0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.0f,
    0.707107f, 1.0f, -0.707107f, 0.0f, 1.0f, 0.0f, 0.853553f, 0.146447f,
    0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f,
    1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.5f,
    0.707107f, -1.0f, 0.707107f, 0.0f, -1.0f, 0.0f, 0.853553f, 0.853553f,
    0.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.5f, 1.0f,
    -0.707107f, -1.0f, 0.707107f, 0.0f, -1.0f, 0.0f, 0.146447f, 0.853553f,
    -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.5f,
    -0.707107f, -1.0f, -0.707107f, 0.0f, -1.0f, 0.0f, 0.146447f, 0.146447f,
    0.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.0f,
    0.707107f, -1.0f, -0.707107f, 0.0f, -1.0f, 0.0f, 0.853553f, 0.146447f,
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.707107f, 1.0f, 0.707107f, 0.707107f, 0.0f, 0.707107f, 0.125f, 1.0f,
    0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f,
    -0.707107f, 1.0f, 0.707107f, -0.707107f, 0.0f, 0.707107f, 0.375f, 1.0f,
    -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.5f, 1.0f,
    -0.707107f, 1.0f, -0.707107f, -0.707107f, 0.0f, -0.707107f, 0.625f, 1.0f,
    0.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.75f, 1.0f,
    0.707107f, 1.0f, -0.707107f, 0.707107f, 0.0f, -0.707107f, 0.875f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.707107f, -1.0f, 0.707107f, 0.707107f, 0.0f, 0.707107f, 0.125f, 0.0f,
    0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.25f, 0.0f,
    -0.707107f, -1.0f, 0.707107f, -0.707107f, 0.0f, 0.707107f, 0.375f, 0.0f,
    -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.5f, 0.0f,
    -0.707107f, -1.0f, -0.707107f, -0.707107f, 0.0f, -0.707107f, 0.625f, 0.0f,
    0.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.75f, 0.0f,
    0.707107f, -1.0f, -0.707107f, 0.707107f, 0.0f, -0.707107f, 0.875f, 0.0f,
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
