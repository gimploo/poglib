#pragma once
#include "./globjects.h"
#include "./common.h"


typedef struct glvertex_t {

    vec3f_t position;
    vec4f_t color;
    vec2f_t texture_coord;
    u8      texture_id;

} glvertex_t ;

typedef struct { glvertex_t vertex[3]; } gltri_t;
typedef struct { glvertex_t vertex[4]; } glquad_t;
typedef struct { glvertex_t vertex[MAX_VERTICES_PER_CIRCLE]; } glcircle_t;

typedef struct {

    glcircle_t  vertices;
    u8          sides;

} glpolygon_t ;

#define MAX_VERTICES_PER_CUBE   72
#define MAX_UVS_PER_CUBE        36


gltri_t         gltri(trif_t tri, vec4f_t color, quadf_t tex_coord, u8 texid);
glquad_t        glquad(const quadf_t pos, const vec4f_t rgba, const quadf_t uv, const u8 tex_id);
glcircle_t      glcircle(circle_t circle, vec4f_t color, quadf_t uv, u8 texid);
glpolygon_t     glpolygon(polygon_t polygon, vec4f_t color, quadf_t uv, u8 texid);

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

    glbatch_t text;

} gltext_t;

gltext_t        gltext_init(const u64 capacity);
#define         gltext_put(PTEXT, ELEM)         queue_put(&(PTEXT)->text.globjs, (ELEM))
#define         gltext_get(PTEXT, ELEM)         queue_get_in_buffer(&(PTEXT)->text.globjs, (ELEM))
#define         gltext_is_empty(PTEXT)          queue_is_empty(&(PTEXT)->text.globjs)
#define         gltext_clear(PTEXT)             queue_clear(&(PTEXT)->text.globjs)
#define         gltext_destroy(PTEXT)           glbatch_destroy(&(PTEXT)->text)


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
                batch->globjs = __impl_queue_init(
                        oldqueue.__capacity,
                        poly->sides * 3 * sizeof(glvertex_t ),
                        "glpolygon_t");
            }

            if ((poly->sides * 3) != batch->__meta.nvertex)
               eprint("Trying to push a polygon of `%i` vertices to a batch expecting polygon of `%u` vertices", 
                       poly->sides,
                       batch->__meta.nvertex);

            const u64 size = 
                sizeof(glvertex_t ) * batch->__meta.nvertex;

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


const u32 DEFAULT_QUAD_INDICES[] = {
    0, 1, 2,
    2, 3, 0
};

const u32 DEFAULT_CUBE_INDICES[] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // right
    1, 5, 6,
    6, 2, 1,
    // back
    7, 6, 5,
    5, 4, 7,
    // left
    4, 0, 3,
    3, 7, 4,
    // bottom
    4, 5, 1,
    1, 0, 4,
    // top
    3, 2, 6,
    6, 7, 3
};

// Creates a quad suited for OpenGL
glquad_t glquad(const quadf_t positions, const vec4f_t color, const quadf_t tex_coord, const u8 tex_id)
{
    return (glquad_t) { 

        .vertex = {
            [TOP_LEFT] = (glvertex_t ){ 
                positions.vertex[0].raw[X], positions.vertex[0].raw[Y], positions.vertex[0].raw[Z], 
                color, 
                tex_coord.vertex[0].raw[X], tex_coord.vertex[0].raw[Y],
                tex_id
            },
            [TOP_RIGHT] = (glvertex_t ){ 
                positions.vertex[1].raw[X], positions.vertex[1].raw[Y], positions.vertex[1].raw[Z], 
                color, 
                tex_coord.vertex[1].raw[X], tex_coord.vertex[1].raw[Y],
                tex_id
            }, 
            [BOTTOM_RIGHT] = (glvertex_t ){ 
                positions.vertex[2].raw[X], positions.vertex[2].raw[Y], positions.vertex[2].raw[Z], 
                color, 
                tex_coord.vertex[2].raw[X], tex_coord.vertex[2].raw[Y],
                tex_id
            }, 
            [BOTTOM_LEFT] = (glvertex_t ){ 
                positions.vertex[3].raw[X], positions.vertex[3].raw[Y], positions.vertex[3].raw[Z], 
                color, 
                tex_coord.vertex[3].raw[X], tex_coord.vertex[3].raw[Y],
                tex_id
            } 
        }
    };
}

gltri_t gltri(trif_t tri, vec4f_t color, quadf_t tex_coord, u8 texid)
{
    return (gltri_t) {

        .vertex[0] = (glvertex_t ){ 
            tri.vertex[0].raw[X], tri.vertex[0].raw[Y], tri.vertex[0].raw[Z], 
            color, 
            tex_coord.vertex[0].raw[X], tex_coord.vertex[0].raw[Y],
            texid
        }, 
        .vertex[1] = (glvertex_t ){ 
            tri.vertex[1].raw[X], tri.vertex[1].raw[Y], tri.vertex[1].raw[Z], 
            color, 
            tex_coord.vertex[1].raw[X], tex_coord.vertex[1].raw[Y],
            texid
        }, 
        .vertex[2] = (glvertex_t ) { 
            tri.vertex[2].raw[X], tri.vertex[2].raw[Y], tri.vertex[2].raw[Z], 
            color, 
            tex_coord.vertex[2].raw[X], tex_coord.vertex[2].raw[Y],
            texid
        }, 
    };
}

glpolygon_t glpolygon(polygon_t polygon, vec4f_t color, quadf_t uv, u8 texid)
{
    glpolygon_t output = {0} ;

    glvertex_t *vertices = output.vertices.vertex;

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
        vertices[i].texture_coord = glms_vec2(uv.vertex[i]);
        vertices[i].texture_id = texid;
    }
    return output;
}

glcircle_t glcircle(circle_t circle, vec4f_t color, quadf_t uv, u8 texid)
{
    glcircle_t output = {0} ;

    glvertex_t *vertices = output.vertex;

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
        vertices[i].texture_coord = glms_vec2(uv.vertex[i]);
        vertices[i].texture_id = texid;
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
        .text =  {
            .globjs = __impl_queue_init(capacity, sizeof(glquad_t ), "glquad_t"),
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
        .globjs   = __impl_queue_init(capacity, typesize, type_name),
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
