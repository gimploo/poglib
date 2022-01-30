#pragma once

#include "./globjects.h"
#include "./common.h"


typedef struct glvertex_t   glvertex_t;
typedef struct gltri_t      gltri_t;
typedef struct glquad_t     glquad_t;
typedef struct glcircle_t   glcircle_t;
typedef struct glbatch_t    glbatch_t;

typedef enum {

    GLBT_gltri_t = 0,
    GLBT_glquad_t,
    GLBT_glcircle_t,
    GLBT_COUNT

} glbatch_type;


gltri_t         gltri_init(trif_t tri, vec4f_t color, quadf_t tex_coord, u8 texid);
glquad_t        glquad_init(quadf_t positions, vec4f_t color, quadf_t tex_coord, u8 tex_id);
glcircle_t      glcircle_init(circle_t circle, vec4f_t color, quadf_t uv, u8 texid);


#define         glbatch_init(CAPACITY, TYPE)    __impl_glbatch_init((CAPACITY), GLBT_type(TYPE), #TYPE, sizeof(TYPE))
#define         glbatch_put(PBATCH, ELEM)       queue_put(&(PBATCH)->globjs, (ELEM))
#define         glbatch_is_empty(PBATCH)        queue_is_empty(&(PBATCH)->globjs)
#define         glbatch_clear(PBATCH)           queue_clear(&(PBATCH)->globjs)
void            glbatch_destroy(glbatch_t *batch);












#ifndef IGNORE_GL_TYPE_IMPLEMENTATION

#define MAX_TRI_INDICES_CAPACITY 3
#define MAX_QUAD_INDICES_CAPACITY 6

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
                positions.vertex[0].cmp[X], positions.vertex[0].cmp[Y], positions.vertex[0].cmp[Z], 
                color, 
                tex_coord.vertex[0].cmp[X], tex_coord.vertex[0].cmp[Y],
                tex_id
            },
            [TOP_RIGHT] = (glvertex_t ){ 
                positions.vertex[1].cmp[X], positions.vertex[1].cmp[Y], positions.vertex[1].cmp[Z], 
                color, 
                tex_coord.vertex[1].cmp[X], tex_coord.vertex[1].cmp[Y],
                tex_id
            }, 
            [BOTTOM_RIGHT] = (glvertex_t ){ 
                positions.vertex[2].cmp[X], positions.vertex[2].cmp[Y], positions.vertex[2].cmp[Z], 
                color, 
                tex_coord.vertex[2].cmp[X], tex_coord.vertex[2].cmp[Y],
                tex_id
            }, 
            [BOTTOM_LEFT] = (glvertex_t ){ 
                positions.vertex[3].cmp[X], positions.vertex[3].cmp[Y], positions.vertex[3].cmp[Z], 
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
            tri.vertex[0].cmp[X], tri.vertex[0].cmp[Y], tri.vertex[0].cmp[Z], 
            color, 
            tex_coord.vertex[0].cmp[X], tex_coord.vertex[0].cmp[Y],
            texid
        }, 
        .vertex[1] = (glvertex_t ){ 
            tri.vertex[1].cmp[X], tri.vertex[1].cmp[Y], tri.vertex[1].cmp[Z], 
            color, 
            tex_coord.vertex[1].cmp[X], tex_coord.vertex[1].cmp[Y],
            texid
        }, 
        .vertex[2] = (glvertex_t ) { 
            tri.vertex[2].cmp[X], tri.vertex[2].cmp[Y], tri.vertex[2].cmp[Z], 
            color, 
            tex_coord.vertex[2].cmp[X], tex_coord.vertex[2].cmp[Y],
            texid
        }, 
    };
}

glcircle_t glcircle_init(circle_t circle, vec4f_t color, quadf_t uv, u8 texid)
{
    glcircle_t output = {0} ;

    glvertex_t *vertices = output.vertex;

    // TODO: Textures on circles
    //for (u64 i = 0; i < MAX_TRIANGLES_PER_CIRCLE; i++)
    //{
        //uv.vertex[i].cmp[X] = (circle.points[i].cmp[X] /circle.radius + 1)*0.5;
        //uv.vertex[i].cmp[Y] = (circle.points[i].cmp[Y]/circle.radius + 1)*0.5;

         //float tx = (x/r + 1)*0.5;
         //float ty = (y/r + 1)*0.5;

    //}

    for (u64 i = 0; i < MAX_VERTICES_PER_CIRCLE; i++)
    {
        vertices[i].position = circle.points[i];
        vertices[i].color = color; 
        vertices[i].texture_coord = vec2f(uv.vertex[i]);
        vertices[i].texture_id = texid;
    }

    return output;
}


#define GLBT_type(TYPE) GLBT_##TYPE

struct glbatch_t {

    queue_t             globjs;
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
glbatch_t __impl_glbatch_init(u64 capacity, glbatch_type type, const char *type_name, u64 type_size)
{
    return (glbatch_t ) {
        .globjs   = __impl_queue_init(capacity, type_size, type_name),
        .type       = type
    };
}



void glbatch_destroy(glbatch_t *batch) 
{
    queue_destroy(&batch->globjs);
}

#endif
