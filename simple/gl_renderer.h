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

typedef struct  {

    vec3f_t position;
    vec3f_t color;
    vec2f_t texture_coord;

} gl_vertex_t;

typedef struct  { gl_vertex_t vertices[3]; } gl_tri_t;
typedef struct  { gl_vertex_t vertices[4]; } gl_quad_t;


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
void                gl_render2d_draw_quad(gl_renderer2d_t *renderer, gl_quad_t quad);
void                gl_render2d_destroy(gl_renderer2d_t *renderer);


/*----------------------------------------------------------
 // Implementations
----------------------------------------------------------*/
gl_renderer2d_t gl_renderer2d_init(const gl_shader_t *shader, const gl_texture2d_t *texture)
{
    if (shader == NULL) eprint("shader argument is null");
    return (gl_renderer2d_t) {
        .shader = shader,
        .texture = texture
    };
}

void gl_render2d_draw_quad(gl_renderer2d_t *renderer, gl_quad_t quad)
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

void gl_render2d_destroy(gl_renderer2d_t *renderer)
{
    if (renderer == NULL) eprint("renderer argument is null");

    shader_destroy(renderer->shader);
    vao_destroy(&renderer->vao);
    if (renderer->texture != NULL) texture_destroy(renderer->texture);
}


#endif //__MY_GL_RENDERER_H__
