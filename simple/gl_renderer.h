#ifndef __MY_GL_RENDERER_H__
#define __MY_GL_RENDERER_H__

#include <GL/glew.h>

#include "../math/shapes.h"

#include "gl/shader.h"
#include "gl/texture2d.h"

#include "gl/VAO.h"
#include "gl/VBO.h"
#include "gl/EBO.h"

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

} gl_vertex_t;

typedef struct  { gl_vertex_t vertices[3]; } gl_tri_t;
typedef struct  { gl_vertex_t vertices[4]; } gl_quad_t;

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
    const gl_shader_t       *shader;
    const gl_texture2d_t    *texture;

} gl_renderer2d_t;

/*--------------------------------------------------------
 // Declarations
--------------------------------------------------------*/

gl_renderer2d_t     gl_renderer2d_init(const gl_shader_t *shader, const gl_texture2d_t *texture);

void                gl_renderer2d_draw_quad(gl_renderer2d_t *renderer, const gl_quad_t quad);
void                gl_renderer2d_draw_from_batch(gl_renderer2d_t *renderer, const gl_batch_t *batch);

void                gl_render2d_destroy(gl_renderer2d_t *renderer);


/*----------------------------------------------------------
 // Implementations
----------------------------------------------------------*/

static inline void __gen_quad_indices(u32 indices[], const u32 shape_count)
{
    const u32 indices_count = shape_count * DEFAULT_QUAD_INDICES_CAPACITY;

    memcpy(indices, DEFAULT_QUAD_INDICES, sizeof(DEFAULT_QUAD_INDICES));
    for (int i = 1; i < shape_count; i++)
    {
        indices[(i*6) + 0]   = DEFAULT_QUAD_INDICES[0] + 4; 
        indices[(i*6) + 1]   = DEFAULT_QUAD_INDICES[1] + 4;
        indices[(i*6) + 2]   = DEFAULT_QUAD_INDICES[2] + 4;
        indices[(i*6) + 3]   = DEFAULT_QUAD_INDICES[3] + 4;
        indices[(i*6) + 4]   = DEFAULT_QUAD_INDICES[4] + 4;
        indices[(i*6) + 5]   = DEFAULT_QUAD_INDICES[5] + 4;
    }

}

static inline void __gen_tri_indices(u32 indices[], const u32 shape_count)
{
    for (int i = 0; i < shape_count; i++)
    {
        indices[i] = 3 * i;
        indices[i+1] = indices[i] + 1;
        indices[i+2] = indices[i+1] + 1;
    }

}


gl_renderer2d_t gl_renderer2d_init(const gl_shader_t *shader, const gl_texture2d_t *texture)
{
    if (shader == NULL) eprint("shader argument is null");
    return (gl_renderer2d_t) {
        .shader = shader,
        .texture = texture
    };
}

void gl_renderer2d_draw_quad(gl_renderer2d_t *renderer, const gl_quad_t quad)
{
    if (renderer == NULL) eprint("renderer argument is null");

    renderer->vao = vao_init(1);
    vbo_t vbo = vbo_init(&quad, sizeof(gl_quad_t));
    ebo_t ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, DEFAULT_QUAD_INDICES_CAPACITY);

    vao_bind(&renderer->vao);

        vao_push(&renderer->vao, &vbo);
            vao_set_attributes(&renderer->vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position));
            vao_set_attributes(&renderer->vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color));

            if (renderer->texture != NULL) {
                vao_set_attributes(&renderer->vao, 0, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord));
                texture_bind(renderer->texture, 0);
            }

            shader_bind(renderer->shader);
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

    renderer->vao = vao_init(1);

    u32 indices_buffer[
        batch->shape_count * (
                    batch->shape_type == BT_QUAD ?  
                    DEFAULT_QUAD_INDICES_CAPACITY : DEFAULT_TRI_INDICES_CAPACITY
                )
    ]; 
    memset(indices_buffer, 0, sizeof(indices_buffer));

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
                texture_bind(renderer->texture, 0);
            }
            shader_bind(renderer->shader);
            vao_draw(&renderer->vao);

        vao_pop(&renderer->vao);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
}


void gl_render2d_destroy(gl_renderer2d_t *renderer)
{
    if (renderer == NULL) eprint("renderer argument is null");

    shader_destroy(renderer->shader);
    vao_destroy(&renderer->vao);
    if (renderer->texture != NULL) texture_destroy(renderer->texture);
}


#endif //__MY_GL_RENDERER_H__
