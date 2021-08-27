#ifndef __MY_GL_RENDERER_2D_H__
#define __MY_GL_RENDERER_2D_H__

#include <GL/glew.h>

#include "../math/shapes.h"

#include "gl/shader.h"
#include "gl/texture2d.h"

#include "gl/VAO.h"
#include "gl/VBO.h"
#include "gl/EBO.h"


#define MAX_QUAD_CAPACITY_PER_DRAW_CALL 10000


#define DEFAULT_TRI_INDICES_CAPACITY 3
const u32 DEFAULT_TRI_INDICES[] = {
    0, 1, 2
};

#define DEFAULT_QUAD_INDICES_CAPACITY 6
const u32 DEFAULT_QUAD_INDICES[] = {
    0, 1, 2,
    2, 3, 0
};

typedef struct  gl_vertex_t {

    vec3f_t position;
    vec3f_t color;
    vec2f_t texture_coord;
    u8      texture_id;

} gl_vertex_t;

typedef struct  { gl_vertex_t vertex[3]; } gl_tri_t;
typedef struct  { gl_vertex_t vertex[4]; } gl_quad_t;

// Creates a quad of a single color assuming no textures
INTERNAL gl_quad_t gl_quad(quadf_t quad, vec3f_t color, quadf_t tex_coord)
{
    return (gl_quad_t) {

        .vertex[0] = (gl_vertex_t ){ 
            quad.vertex[0].cmp[X], quad.vertex[0].cmp[Y], 0.0f, 
            color, 
            0.0f, 0.0f,
            0
        }, 
        .vertex[1] = (gl_vertex_t ){ 
            quad.vertex[1].cmp[X], quad.vertex[1].cmp[Y], 0.0f, 
            color, 
            0.0f, 0.0f,
            0
        }, 
        .vertex[2] = (gl_vertex_t ) { 
            quad.vertex[2].cmp[X], quad.vertex[2].cmp[Y], 0.0f, 
            color, 
            0.0f, 0.0f,
            0
        }, 
        .vertex[3] = (gl_vertex_t ){ 
            quad.vertex[3].cmp[X], quad.vertex[3].cmp[Y], 0.0f, 
            color, 
            0.0f, 0.0f,
            0
        }, 
    };
}

INTERNAL gl_tri_t gl_tri(trif_t tri, vec3f_t color, quadf_t tex_coord)
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

    BT_TRI = 0,
    BT_QUAD,
    BT_COUNT

} gl_batch_type;

typedef struct gl_batch_t {

    const gl_batch_type shape_type;
    const u64           shape_count;

    const gl_vertex_t   *vertex_buffer;
    const u64           vertex_buffer_size;


} gl_batch_t;


/*==================================================
 // OpenGL 2D renderer
==================================================*/

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

void                gl_render2d_destroy(gl_renderer2d_t *renderer);


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
{    if (renderer == NULL) eprint("renderer argument is null");

    vbo_t vbo; 
    ebo_t ebo;

    vao_bind(&renderer->vao);

        vbo = vbo_init(&tri, sizeof(gl_tri_t));
        ebo = ebo_init(&vbo, DEFAULT_TRI_INDICES, DEFAULT_TRI_INDICES_CAPACITY);

        vao_push(&renderer->vao, &vbo);
            vao_set_attributes(&renderer->vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position));
            vao_set_attributes(&renderer->vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color));

            if (renderer->texture != NULL) {
                vao_set_attributes(&renderer->vao, 0, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord));
                vao_set_attributes(&renderer->vao, 0, 1, GL_UNSIGNED_INT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_id));
                gl_texture_bind(renderer->texture, 0);
            }

            gl_shader_bind(renderer->shader);
            vao_draw(&renderer->vao);
        vao_pop(&renderer->vao);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);


}

//NOTE: make sure to not have texture uniform if your passing NULL as texture argument
gl_renderer2d_t gl_renderer2d_init(gl_shader_t *shader, const gl_texture2d_t *texture)
{
    if (shader == NULL) eprint("shader argument is null");
    return (gl_renderer2d_t) {
        .vao = vao_init(1),
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
        ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, DEFAULT_QUAD_INDICES_CAPACITY);

        vao_push(&renderer->vao, &vbo);
            vao_set_attributes(&renderer->vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position));
            vao_set_attributes(&renderer->vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color));

            if (renderer->texture != NULL) {
                vao_set_attributes(&renderer->vao, 0, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord));
                vao_set_attributes(&renderer->vao, 0, 1, GL_UNSIGNED_INT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_id));
                gl_texture_bind(renderer->texture, 0);
            }

            gl_shader_bind(renderer->shader);
            vao_draw(&renderer->vao);
        vao_pop(&renderer->vao);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);

}


void gl_renderer2d_draw_from_batch(gl_renderer2d_t *renderer, const gl_batch_t *batch) 
{
    if (renderer == NULL) eprint("renderer argument is null");
    if (batch == NULL) eprint("batch argument is null");

    ebo_t ebo;
    vbo_t vbo;

    u32 indices_buffer[
        batch->shape_count * (
                    batch->shape_type == BT_QUAD ?  
                    DEFAULT_QUAD_INDICES_CAPACITY : DEFAULT_TRI_INDICES_CAPACITY)
    ]; 
    memset(indices_buffer, 0, sizeof(indices_buffer));

    GL_LOG("Batch size: %li\n", batch->vertex_buffer_size);

    vao_bind(&renderer->vao);

        vbo = vbo_init(batch->vertex_buffer, batch->vertex_buffer_size);

        switch(batch->shape_type)
        {
            case BT_QUAD:
                __gen_quad_indices(indices_buffer, batch->shape_count);
                ebo = ebo_init(&vbo, indices_buffer, DEFAULT_QUAD_INDICES_CAPACITY * batch->shape_count);
                break;
            case BT_TRI:
                __gen_tri_indices(indices_buffer, batch->shape_count);
                ebo = ebo_init(&vbo, indices_buffer, DEFAULT_TRI_INDICES_CAPACITY * batch->shape_count);
                break;
            default:
                eprint("gl_batch_type: TYPE = (%i) unknown\n", batch->shape_type);
        }

        vao_push(&renderer->vao, &vbo);

            vao_set_attributes(&renderer->vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position));
            vao_set_attributes(&renderer->vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color));

            if (renderer->texture != NULL) {
                vao_set_attributes(&renderer->vao, 0, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord));
                vao_set_attributes(&renderer->vao, 0, 1, GL_UNSIGNED_INT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_id));
                gl_texture_bind(renderer->texture, 0);
            }
            gl_shader_bind(renderer->shader);
            vao_draw(&renderer->vao);

        vao_pop(&renderer->vao);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
}


void gl_render2d_destroy(gl_renderer2d_t *renderer)
{
    if (renderer == NULL) eprint("renderer argument is null");

    gl_shader_destroy(renderer->shader);
    vao_destroy(&renderer->vao);
    if (renderer->texture != NULL) gl_texture_destroy(renderer->texture);
}


#endif //__MY_GL_RENDERER_2D_H__
