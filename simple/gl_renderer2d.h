#ifndef __MY_GL_RENDERER_2D_H__
#define __MY_GL_RENDERER_2D_H__

#include <GL/glew.h>

#include "../math/shapes.h"
#include "gl/shader.h"
#include "gl/texture2d.h"
#include "gl/VAO.h"
#include "gl/VBO.h"
#include "gl/EBO.h"
#include "gl/framebuffer.h"

/*====================================================================================
 // OpenGL specific types (Vertex, Triangles and Quads) and custom batch type
====================================================================================*/

typedef struct  gl_vertex_t {

    vec3f_t position;
    vec3f_t color;
    vec2f_t texture_coord;
    u8      texture_id;

} gl_vertex_t;

typedef struct  { gl_vertex_t vertex[3]; } gl_tri_t;
typedef struct  { gl_vertex_t vertex[4]; } gl_quad_t;

// Creates a quad suited for OpenGL
gl_quad_t gl_quad(quadf_t positions, vec3f_t color, quadf_t tex_coord)
{
    return (gl_quad_t) {

        .vertex[TOP_LEFT] = (gl_vertex_t ){ 
            positions.vertex[0].cmp[X], positions.vertex[0].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[0].cmp[X], tex_coord.vertex[0].cmp[Y],
            0
        }, 
        .vertex[TOP_RIGHT] = (gl_vertex_t ){ 
            positions.vertex[1].cmp[X], positions.vertex[1].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[1].cmp[X], tex_coord.vertex[1].cmp[Y],
            0
        }, 
        .vertex[BOTTOM_RIGHT] = (gl_vertex_t ){ 
            positions.vertex[2].cmp[X], positions.vertex[2].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[2].cmp[X], tex_coord.vertex[2].cmp[Y],
            0
        }, 
        .vertex[BOTTOM_LEFT] = (gl_vertex_t ){ 
            positions.vertex[3].cmp[X], positions.vertex[3].cmp[Y], 0.0f, 
            color, 
            tex_coord.vertex[3].cmp[X], tex_coord.vertex[3].cmp[Y],
            0
        }, 
    };
}

gl_tri_t gl_tri(trif_t tri, vec3f_t color, quadf_t tex_coord)
{
    return (gl_tri_t) {

        .vertex[0] = (gl_vertex_t ){ 
            tri.vertex[0].cmp[X], tri.vertex[0].cmp[Y], 0.0f, 
            color, 
            0.0f, 0.0f,
            0
        }, 
        .vertex[1] = (gl_vertex_t ){ 
            tri.vertex[1].cmp[X], tri.vertex[1].cmp[Y], 0.0f, 
            color, 
            0.0f, 0.0f,
            0
        }, 
        .vertex[2] = (gl_vertex_t ) { 
            tri.vertex[2].cmp[X], tri.vertex[2].cmp[Y], 0.0f, 
            color, 
            0.0f, 0.0f,
            0
        }, 
    };
}

typedef enum {

    BT_gl_tri_t = 0,
    BT_gl_quad_t,
    BT_COUNT

} gl_batch_type;

#define BT_type(TYPE) BT_##TYPE

typedef struct gl_batch_t {


    u32 __indices_buffer[KB];

    const gl_batch_type shape_type;
    u64                 shape_count;

    const gl_vertex_t   *vertex_buffer;
    const u64           vertex_buffer_size;


    ebo_t ebo;
    vbo_t vbo;
    vao_t vao;


} gl_batch_t;

#define gl_batch_init(VERTEX_ARRAY, TYPE) __impl_gl_batch_init((gl_vertex_t *)VERTEX_ARRAY, sizeof(VERTEX_ARRAY), BT_type(TYPE))
#define gl_batch_destroy(PBATCH) do {\
\
    ebo_destroy(&(PBATCH)->ebo);\
    vbo_destroy(&(PBATCH)->vbo);\
    vao_destroy(&(PBATCH)->vao);\
\
} while(0)

/*==================================================
 // OpenGL 2D renderer
==================================================*/

#define MAX_QUAD_CAPACITY_PER_DRAW_CALL 10000
#define DEFAULT_TRI_INDICES_CAPACITY 3
#define DEFAULT_QUAD_INDICES_CAPACITY 6

const u32 DEFAULT_TRI_INDICES[] = {
    0, 1, 2
};

const u32 DEFAULT_QUAD_INDICES[] = {
    0, 1, 2,
    2, 3, 0
};

typedef struct gl_renderer2d_t {

    vao_t                   vao;
    gl_shader_t             *shader;
    const gl_texture2d_t    *texture;

} gl_renderer2d_t;

/*--------------------------------------------------------
 // Declarations
--------------------------------------------------------*/

gl_renderer2d_t     gl_renderer2d_init(gl_shader_t *shader, const gl_texture2d_t *texture);

void                gl_renderer2d_draw_quad(gl_renderer2d_t *renderer, const gl_quad_t quad);
void                gl_renderer2d_draw_triangle(gl_renderer2d_t *renderer, const gl_tri_t tri);
void                gl_renderer2d_draw_from_batch(gl_renderer2d_t *renderer, const gl_batch_t *batch);

void                gl_renderer2d_draw_frame_buffer(gl_framebuffer_t *fbo, const gl_quad_t quad);

//NOTE: renderer destroy only frees the vao in it and not the shaders and textures passed to it
void                gl_renderer2d_destroy(gl_renderer2d_t *renderer);


/*----------------------------------------------------------
 // Implementations
----------------------------------------------------------*/


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
    for (u32 i = 0; i < shape_count; i++)
    {
        indices[i] = 3 * i;
        indices[i+1] = indices[i] + 1;
        indices[i+2] = indices[i+1] + 1;
    }

}

void gl_renderer2d_draw_triangle(gl_renderer2d_t *renderer, const gl_tri_t tri)
{    
    if (renderer == NULL) eprint("renderer argument is null");

    vbo_t vbo; 
    ebo_t ebo;

    vao_bind(&renderer->vao);

        vbo = vbo_init(&tri, sizeof(gl_tri_t));
        ebo = ebo_init(DEFAULT_TRI_INDICES, DEFAULT_TRI_INDICES_CAPACITY);
        vbo.indices_count = ebo_get_count(&ebo);

            vao_set_attributes(&renderer->vao, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position), &vbo);
            vao_set_attributes(&renderer->vao, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color), &vbo);

            if (renderer->texture != NULL) {
                vao_set_attributes(&renderer->vao, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord), &vbo);
                vao_set_attributes(&renderer->vao, 1, GL_UNSIGNED_INT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_id), &vbo);
                gl_texture2d_bind(renderer->texture, 0);
            }

            gl_shader_bind(renderer->shader);
            vao_draw(&renderer->vao, &vbo);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);


}

//NOTE: make sure to not have texture uniform if your passing NULL as texture argument
gl_renderer2d_t gl_renderer2d_init(gl_shader_t *shader, const gl_texture2d_t *texture)
{
    if (shader == NULL) eprint("shader argument is null");
    return (gl_renderer2d_t) {
        .vao = vao_init(),
        .shader = shader,
        .texture = texture
    };
}


void gl_renderer2d_draw_quad(gl_renderer2d_t *renderer, const gl_quad_t quad)
{
    if (renderer == NULL) eprint("renderer argument is null");

    vbo_t vbo; 
    ebo_t ebo;

    vao_bind(&renderer->vao);

        vbo = vbo_init(&quad, sizeof(gl_quad_t));
        ebo = ebo_init(DEFAULT_QUAD_INDICES, DEFAULT_QUAD_INDICES_CAPACITY);
        vbo.indices_count = ebo_get_count(&ebo);

        vbo_bind(&vbo);
        vao_set_attributes(&renderer->vao, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position), &vbo);
        vao_set_attributes(&renderer->vao, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color), &vbo);

        if (renderer->texture != NULL) {
            vao_set_attributes(&renderer->vao, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord), &vbo);
            vao_set_attributes(&renderer->vao, 1, GL_UNSIGNED_INT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_id), &vbo);
            gl_texture2d_bind(renderer->texture, 0);
        }

        gl_shader_bind(renderer->shader);
        vao_draw(&renderer->vao, &vbo);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);

}


gl_batch_t __impl_gl_batch_init(gl_vertex_t vertices[], const u64 vertices_size, gl_batch_type type)
{

    gl_batch_t batch = {
        .shape_type         = type,
        .vertex_buffer      = vertices,
        .vertex_buffer_size = vertices_size,
    };


    batch.vao = vao_init();
    vao_bind(&batch.vao);

    switch(type)
    {
        case BT_gl_quad_t:
            batch.shape_count = (vertices_size / (sizeof(gl_vertex_t))) / 4;
            __gen_quad_indices(batch.__indices_buffer, batch.shape_count);
            batch.ebo = ebo_init(batch.__indices_buffer, DEFAULT_QUAD_INDICES_CAPACITY * batch.shape_count);
            break;
        case BT_gl_tri_t:
            batch.shape_count = (vertices_size / (sizeof(gl_vertex_t))) / 3;
            __gen_tri_indices(batch.__indices_buffer, batch.shape_count);
            batch.ebo = ebo_init(batch.__indices_buffer, DEFAULT_TRI_INDICES_CAPACITY * batch.shape_count);
            break;
        default:
            eprint("gl_batch_type: TYPE = (%i) unknown\n", type);
    }


    batch.vbo = vbo_init(vertices, vertices_size);
    batch.vbo.indices_count = ebo_get_count(&batch.ebo);

    vao_set_attributes(&batch.vao, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position), &batch.vbo);
    vao_set_attributes(&batch.vao, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color), &batch.vbo);
    vao_set_attributes(&batch.vao, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord), &batch.vbo);
    vao_set_attributes(&batch.vao, 1, GL_UNSIGNED_INT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_id), &batch.vbo);

    return batch;
}

void gl_renderer2d_draw_from_batch(gl_renderer2d_t *renderer, const gl_batch_t *batch) 
{
    if (renderer == NULL) eprint("renderer argument is null");
    if (batch == NULL) eprint("batch argument is null");

    vao_t *vao = (vao_t *)&batch->vao;
    vbo_t *vbo = (vbo_t *)&batch->vbo;
    ebo_t *ebo = (ebo_t *)&batch->ebo;

    GL_LOG("Batch size: %li\n", batch->vertex_buffer_size);

    vao_bind(vao);

        if (renderer->texture != NULL) {
            gl_texture2d_bind(renderer->texture, 0);
        }
        gl_shader_bind(renderer->shader);
        vao_draw(vao, vbo);

    vao_unbind();
}


void gl_renderer2d_destroy(gl_renderer2d_t *renderer)
{
    if (renderer == NULL) eprint("renderer argument is null");

    // vao
    vao_destroy(&renderer->vao);
}

void gl_renderer2d_draw_frame_buffer(gl_framebuffer_t *fbo, const gl_quad_t quad)
{
    gl_renderer2d_t rd = gl_renderer2d_init(&fbo->color_shader, &fbo->color_texture);
    gl_renderer2d_draw_quad(&rd, quad);
    gl_renderer2d_destroy(&rd);
}

#endif //__MY_GL_RENDERER_2D_H__
