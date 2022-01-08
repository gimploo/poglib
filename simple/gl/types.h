#pragma once

#include "../../math/shapes.h"
#include "VAO.h"
#include "./_common.h"


typedef struct glvertex_t   glvertex_t;
typedef struct gltri_t      gltri_t;
typedef struct glquad_t     glquad_t;
typedef struct glbatch_t    glbatch_t;

typedef enum {

    GLBT_gltri_t = 0,
    GLBT_glquad_t,
    GLBT_COUNT

} glbatch_type;


gltri_t     gltri_init(trif_t tri, vec4f_t color, quadf_t tex_coord, u8 texid);
glquad_t    glquad_init(quadf_t positions, vec4f_t color, quadf_t tex_coord, u8 tex_id);


#define     glbatch_init(CAPACITY, TYPE)    __impl_glbatch_init((CAPACITY), GLBT_type(TYPE), sizeof(TYPE))
#define     glbatch_put(PBATCH, ELEM)       queue_put(&(PBATCH)->vertices, (ELEM))
void        glbatch_destroy(glbatch_t *batch);












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

    glvertex_t  vertex[MAX_VERTICES_PER_CIRCLE];
};

// Creates a quad suited for OpenGL
glquad_t glquad_init(quadf_t positions, vec4f_t color, quadf_t tex_coord, u8 tex_id)
{
    return (glquad_t) { 

        .vertex = {
            [TOP_LEFT] = (glvertex_t ){ 
                positions.vertex[0].cmp[X], positions.vertex[0].cmp[Y], 0.0f, 
                color, 
                tex_coord.vertex[0].cmp[X], tex_coord.vertex[0].cmp[Y],
                tex_id
            },
            [TOP_RIGHT] = (glvertex_t ){ 
                positions.vertex[1].cmp[X], positions.vertex[1].cmp[Y], 0.0f, 
                color, 
                tex_coord.vertex[1].cmp[X], tex_coord.vertex[1].cmp[Y],
                tex_id
            }, 
            [BOTTOM_RIGHT] = (glvertex_t ){ 
                positions.vertex[2].cmp[X], positions.vertex[2].cmp[Y], 0.0f, 
                color, 
                tex_coord.vertex[2].cmp[X], tex_coord.vertex[2].cmp[Y],
                tex_id
            }, 
            [BOTTOM_LEFT] = (glvertex_t ){ 
                positions.vertex[3].cmp[X], positions.vertex[3].cmp[Y], 0.0f, 
                color, 
                tex_coord.vertex[3].cmp[X], tex_coord.vertex[3].cmp[Y],
                tex_id
            } 
        }
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

    queue_t             vertices;
    glbatch_type        type;

};


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

void __gen_tri_indices(u32 indices[], const u32 shape_count)
{
    memcpy(indices, DEFAULT_TRI_INDICES, sizeof(DEFAULT_TRI_INDICES));
    for (u64 i = 1; i < shape_count; i++)
    {
        indices[(i*3) + 0]   = DEFAULT_TRI_INDICES[0] + (3 * i); 
        indices[(i*3) + 1]   = DEFAULT_TRI_INDICES[1] + (3 * i);
        indices[(i*3) + 2]   = DEFAULT_TRI_INDICES[2] + (3 * i);
    }

}
glbatch_t __impl_glbatch_init(u64 capacity, glbatch_type type, u64 type_size)
{
    return (glbatch_t ) {
        .vertices = __impl_queue_init(capacity, type_size),
        .type = type
    };
}


void glbatch_destroy(glbatch_t *batch) 
{
    queue_destroy(&batch->vertices);
}

#endif
