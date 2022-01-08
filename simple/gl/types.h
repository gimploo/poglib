#pragma once

#include "../../math/shapes.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"


typedef struct glvertex_t   glvertex_t;
typedef struct gltri_t      gltri_t;
typedef struct glcircle_t   glcircle_t; 
typedef struct glquad_t     glquad_t;
typedef struct glbatch_t    glbatch_t;

typedef enum {

    GLBT_gltri_t = 0,
    GLBT_glquad_t,
    GLBT_glcircle_t,
    GLBT_COUNT

} glbatch_type;


gltri_t     gltri_init(trif_t tri, vec4f_t color, quadf_t tex_coord, u8 texid);
glquad_t    glquad_init(quadf_t positions, vec4f_t color, quadf_t tex_coord, u8 tex_id);
glcircle_t  glcircle_init(circle_t circle, vec4f_t color, quadf_t uv, u8 texid);

#define     glbatch_init(PVERTICES, SIZE, TYPE) __impl_gl_batch_init((glvertex_t *)PVERTICES, SIZE, GLBT_type(TYPE))
#define     glbatch_destroy(PBATCH) do {\
\
    ebo_destroy(&(PBATCH)->ebo);\
    vbo_destroy(&(PBATCH)->vbo);\
    vao_destroy(&(PBATCH)->vao);\
\
} while(0)












#ifndef IGNORE_GL_TYPE_IMPLEMENTATION

#define DEFAULT_TRI_INDICES_CAPACITY 3
#define DEFAULT_QUAD_INDICES_CAPACITY 6

const u32 DEFAULT_TRI_INDICES[] = {
    0, 1, 2
};


const u32 DEFAULT_QUAD_INDICES[] = {
    0, 1, 2,
    2, 3, 0
};

struct glvertex_t {

    vec3f_t position;
    vec4f_t color;
    vec2f_t texture_coord;
    u8      texture_id;

} ;


struct gltri_t { 

    glvertex_t vertex[3]; 

};

struct glquad_t { 

    glvertex_t vertex[4]; 

};

struct glcircle_t {

    glvertex_t  vertices[MAX_VERTICES_PER_CIRCLE];
    u64         indices[MAX_VERTICES_PER_CIRCLE];
};

// Creates a quad suited for OpenGL
glquad_t glquad_init(quadf_t positions, vec4f_t color, quadf_t tex_coord, u8 tex_id)
{
    return (glquad_t) {

        .vertex[TOP_LEFT] = (glvertex_t ){ 
            positions.vertex[0].cmp[X], positions.vertex[0].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[0].cmp[X], tex_coord.vertex[0].cmp[Y],
            tex_id
        }, 
        .vertex[TOP_RIGHT] = (glvertex_t ){ 
            positions.vertex[1].cmp[X], positions.vertex[1].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[1].cmp[X], tex_coord.vertex[1].cmp[Y],
            tex_id
        }, 
        .vertex[BOTTOM_RIGHT] = (glvertex_t ){ 
            positions.vertex[2].cmp[X], positions.vertex[2].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[2].cmp[X], tex_coord.vertex[2].cmp[Y],
            tex_id
        }, 
        .vertex[BOTTOM_LEFT] = (glvertex_t ){ 
            positions.vertex[3].cmp[X], positions.vertex[3].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[3].cmp[X], tex_coord.vertex[3].cmp[Y],
            tex_id
        }, 
    };
}

gltri_t gltri_init(trif_t tri, vec4f_t color, quadf_t tex_coord, u8 texid)
{
    return (gltri_t) {

        .vertex[0] = (glvertex_t ){ 
            tri.vertex[0].cmp[X], tri.vertex[0].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[0].cmp[X], tex_coord.vertex[0].cmp[Y],
            texid
        }, 
        .vertex[1] = (glvertex_t ){ 
            tri.vertex[1].cmp[X], tri.vertex[1].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[1].cmp[X], tex_coord.vertex[1].cmp[Y],
            texid
        }, 
        .vertex[2] = (glvertex_t ) { 
            tri.vertex[2].cmp[X], tri.vertex[2].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[2].cmp[X], tex_coord.vertex[2].cmp[Y],
            texid
        }, 
    };
}


#define GLBT_type(TYPE) GLBT_##TYPE

struct glbatch_t {


    u32 __indices_buffer[KB];

    const glbatch_type shape_type;
    u64                 shape_count;

    const glvertex_t   *vertex_buffer;
    const u64           vertex_buffer_size;


    ebo_t ebo;
    vbo_t vbo;
    vao_t vao;


};


INTERNAL void __gen_quad_indices(u32 indices[], const u32 shape_count)
{
    memcpy(indices, DEFAULT_QUAD_INDICES, sizeof(DEFAULT_QUAD_INDICES));
    for (u32 i = 1; i < shape_count; i++)
    {
        indices[(i*6) + 0]   = DEFAULT_QUAD_INDICES[0] + (4 * i); 
        indices[(i*6) + 1]   = DEFAULT_QUAD_INDICES[1] + (4 * i);
        indices[(i*6) + 2]   = DEFAULT_QUAD_INDICES[2] + (4 * i);
        indices[(i*6) + 3]   = DEFAULT_QUAD_INDICES[3] + (4 * i);
        indices[(i*6) + 4]   = DEFAULT_QUAD_INDICES[4] + (4 * i);
        indices[(i*6) + 5]   = DEFAULT_QUAD_INDICES[5] + (4 * i);
    }

}

INTERNAL void __gen_tri_indices(u32 indices[], const u32 shape_count)
{
    memcpy(indices, DEFAULT_TRI_INDICES, sizeof(DEFAULT_TRI_INDICES));
    for (u32 i = 1; i < shape_count; i++)
    {
        indices[(i*3) + 0]   = DEFAULT_TRI_INDICES[0] + (3 * i); 
        indices[(i*3) + 1]   = DEFAULT_TRI_INDICES[1] + (3 * i);
        indices[(i*3) + 2]   = DEFAULT_TRI_INDICES[2] + (3 * i);
    }

}

glbatch_t __impl_gl_batch_init(glvertex_t vertices[], const u64 vertices_size, glbatch_type type)
{

    glbatch_t batch = {
        .shape_type         = type,
        .vertex_buffer      = vertices,
        .vertex_buffer_size = vertices_size,
    };


    batch.vao = vao_init();
    vao_bind(&batch.vao);

    switch(type)
    {
        case GLBT_glquad_t:
            batch.shape_count = (vertices_size / (sizeof(glvertex_t))) / 4;
            __gen_quad_indices(batch.__indices_buffer, batch.shape_count);
            batch.ebo = ebo_init(batch.__indices_buffer, DEFAULT_QUAD_INDICES_CAPACITY * batch.shape_count);
            break;
        case GLBT_gltri_t:
            batch.shape_count = (vertices_size / (sizeof(glvertex_t))) / 3;
            __gen_tri_indices(batch.__indices_buffer, batch.shape_count);
            batch.ebo = ebo_init(batch.__indices_buffer, DEFAULT_TRI_INDICES_CAPACITY * batch.shape_count);
            break;
        default:
            eprint("glbatch_type: TYPE = (%i) unknown\n", type);
    }


    batch.vbo = vbo_init(vertices, vertices_size);
    batch.vbo.indices_count = ebo_get_count(&batch.ebo);

    vao_set_attributes(&batch.vao, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position), &batch.vbo);
    vao_set_attributes(&batch.vao, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color), &batch.vbo);
    vao_set_attributes(&batch.vao, 2, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_coord), &batch.vbo);
    vao_set_attributes(&batch.vao, 1, GL_UNSIGNED_INT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_id), &batch.vbo);

    return batch;
}

glcircle_t glcircle_init(circle_t circle, vec4f_t color, quadf_t uv, u8 texid)
{
    glcircle_t output = {0} ;

    glvertex_t *vertices = output.vertices;
    vec3f_t center = circle.points[0];

    for (u64 i = 0; i < MAX_VERTICES_PER_CIRCLE; i++)
    {
        vertices[i].position = circle.points[i]; 
        vertices[i].color = color; 
        vertices[i].texture_coord = vec2f(0.0f);
        vertices[i].texture_id = 0;
    }

    __gen_tri_indices(output.indices, MAX_VERTICES_PER_CIRCLE / 3);

    return output;
}

#endif
