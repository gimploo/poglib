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
#include "gl/types.h"


/*==================================================
 // OpenGL 2D renderer
==================================================*/

#define MAX_QUAD_CAPACITY_PER_DRAW_CALL 10000

typedef struct glrenderer2d_t {

    vao_t                   vao;
    gl_shader_t             *shader;
    const gl_texture2d_t    *texture;

} glrenderer2d_t;

/*--------------------------------------------------------
 // Declarations
--------------------------------------------------------*/

glrenderer2d_t     glrenderer2d_init(gl_shader_t *shader, const gl_texture2d_t *texture);

void                glrenderer2d_draw_quad(glrenderer2d_t *renderer, const glquad_t quad);
void                glrenderer2d_draw_triangle(glrenderer2d_t *renderer, const gltri_t tri);
void                glrenderer2d_draw_from_batch(glrenderer2d_t *renderer, const glbatch_t *batch);

void                glrenderer2d_draw_frame_buffer(gl_framebuffer_t *fbo, const glquad_t quad);

//NOTE: renderer destroy only frees the vao in it and not the shaders and textures passed to it
void                glrenderer2d_destroy(glrenderer2d_t *renderer);


/*----------------------------------------------------------
 // Implementations
----------------------------------------------------------*/



void glrenderer2d_draw_triangle(glrenderer2d_t *renderer, const gltri_t tri)
{    
    if (renderer == NULL) eprint("renderer argument is null");

    vbo_t vbo; 
    ebo_t ebo;

    vao_bind(&renderer->vao);

        vbo = vbo_init(&tri, sizeof(gltri_t));
        ebo = ebo_init(DEFAULT_TRI_INDICES, DEFAULT_TRI_INDICES_CAPACITY);
        vbo.indices_count = ebo_get_count(&ebo);

            vao_set_attributes(&renderer->vao, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position), &vbo);
            vao_set_attributes(&renderer->vao, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color), &vbo);

            if (renderer->texture != NULL) {
                vao_set_attributes(&renderer->vao, 2, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_coord), &vbo);
                vao_set_attributes(&renderer->vao, 1, GL_UNSIGNED_INT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_id), &vbo);
                gl_texture2d_bind(renderer->texture, 0);
            }

            gl_shader_bind(renderer->shader);
            vao_draw(&renderer->vao, &vbo);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);


}

//NOTE: make sure to not have texture uniform if your passing NULL as texture argument
glrenderer2d_t glrenderer2d_init(gl_shader_t *shader, const gl_texture2d_t *texture)
{
    if (shader == NULL) eprint("shader argument is null");
    return (glrenderer2d_t) {
        .vao = vao_init(),
        .shader = shader,
        .texture = texture
    };
}


void glrenderer2d_draw_quad(glrenderer2d_t *renderer, const glquad_t quad)
{
    if (renderer == NULL) eprint("renderer argument is null");

    vbo_t vbo; 
    ebo_t ebo;

    vao_bind(&renderer->vao);

        vbo = vbo_init(&quad, sizeof(glquad_t));
        ebo = ebo_init(DEFAULT_QUAD_INDICES, DEFAULT_QUAD_INDICES_CAPACITY);
        vbo.indices_count = ebo_get_count(&ebo);

        vbo_bind(&vbo);
        vao_set_attributes(&renderer->vao, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position), &vbo);
        vao_set_attributes(&renderer->vao, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color), &vbo);

        if (renderer->texture != NULL) {
            vao_set_attributes(&renderer->vao, 2, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_coord), &vbo);
            vao_set_attributes(&renderer->vao, 1, GL_UNSIGNED_INT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_id), &vbo);
            gl_texture2d_bind(renderer->texture, 0);
        }

        gl_shader_bind(renderer->shader);
        vao_draw(&renderer->vao, &vbo);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);

}



void glrenderer2d_draw_from_batch(glrenderer2d_t *renderer, const glbatch_t *batch) 
{
    if (renderer == NULL) eprint("renderer argument is null");
    if (batch == NULL) eprint("batch argument is null");

    vao_t *vao = (vao_t *)&batch->vao;
    vbo_t *vbo = (vbo_t *)&batch->vbo;

    GL_LOG("Batch size: %li\n", batch->vertex_buffer_size);

    vao_bind(vao);

        if (renderer->texture != NULL) {
            gl_texture2d_bind(renderer->texture, 0);
        }
        gl_shader_bind(renderer->shader);
        vao_draw(vao, vbo);

    vao_unbind();
}


void glrenderer2d_destroy(glrenderer2d_t *renderer)
{
    if (renderer == NULL) eprint("renderer argument is null");

    // vao
    vao_destroy(&renderer->vao);
}

void glrenderer2d_draw_frame_buffer(gl_framebuffer_t *fbo, const glquad_t quad)
{
    glrenderer2d_t rd = glrenderer2d_init(&fbo->color_shader, &fbo->color_texture);
    glrenderer2d_draw_quad(&rd, quad);
    glrenderer2d_destroy(&rd);
}

#endif //__MY_GL_RENDERER_2D_H__
