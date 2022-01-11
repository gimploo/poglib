#pragma once
#include <GL/glew.h>
#include "../math/shapes.h"
#include "gl/shader.h"
#include "gl/texture2d.h"
#include "gl/globjects.h"
#include "gl/framebuffer.h"
#include "gl/types.h"




typedef struct glrenderer2d_t glrenderer2d_t ; 


glrenderer2d_t      glrenderer2d_init(glshader_t *shader, gltexture2d_t *texture);

void                glrenderer2d_draw_quad(glrenderer2d_t *renderer, const glquad_t quad);
void                glrenderer2d_draw_triangle(glrenderer2d_t *renderer, const gltri_t tri);
void                glrenderer2d_draw_circle(glrenderer2d_t *renderer, const glcircle_t circle);

void                glrenderer2d_draw_from_batch(glrenderer2d_t *renderer, const glbatch_t *batch);
void                glrenderer2d_draw_frame_buffer(glframebuffer_t *fbo, const glquad_t quad);

#define             glrenderer2d_destroy(PREND)    vao_destroy(&(PREND)->__vao)









#ifndef IGNORE_GLRENDERER2D_IMPLEMENTATION

#define MAX_QUAD_CAPACITY_PER_DRAW_CALL 10000

global u32 GLOBAL_BATCH_POLY_INDICIES_BUFFER[MAX_QUAD_CAPACITY_PER_DRAW_CALL * MAX_VERTICES_PER_CIRCLE];

global u32 GLOBAL_POLY_INDICIES_BUFFER[MAX_VERTICES_PER_CIRCLE];

struct glrenderer2d_t {

    vao_t         __vao;
    glshader_t    *__shader;
    gltexture2d_t *__texture;

};


//NOTE: make sure to not have texture uniform if your passing NULL as texture argument
glrenderer2d_t glrenderer2d_init(glshader_t *shader, gltexture2d_t *texture)
{
    return (glrenderer2d_t ) {
        .__vao = vao_init(),
        .__shader = shader,
        .__texture = texture,
    };

}

void glrenderer2d_draw_triangle(glrenderer2d_t *renderer, const gltri_t tri)
{    
    if (renderer == NULL) eprint("renderer argument is null");

    vao_t *vao = &renderer->__vao;
    vbo_t vbo; 

    vao_bind(vao);

            vbo = vbo_init(tri.vertex);
            vao_set_attributes(vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position));
            vao_set_attributes(vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));

            if (renderer->__texture != NULL) {
                vao_set_attributes(vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_coord));
                vao_set_attributes(vao, &vbo,1, GL_UNSIGNED_INT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_id));
                gltexture2d_bind(renderer->__texture, 0);
            }

            glshader_bind((glshader_t *)renderer->__shader);
            vao_draw_with_vbo(&renderer->__vao, &vbo);

    vao_unbind();

    vbo_destroy(&vbo);

}

void glrenderer2d_draw_circle(glrenderer2d_t *renderer, const glcircle_t circle)
{    
    if (renderer == NULL) eprint("renderer argument is null");

    vao_t *vao = &renderer->__vao;
    vbo_t vbo; 
    ebo_t ebo;

    vao_bind(vao);

            vbo = vbo_init(circle.vertex);
            vao_set_attributes(vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position));
            vao_set_attributes(vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));

            if (renderer->__texture != NULL) {
                vao_set_attributes(vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_coord));
                vao_set_attributes(vao, &vbo,1, GL_UNSIGNED_INT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_id));
                gltexture2d_bind(renderer->__texture, 0);
            }

            glshader_bind((glshader_t *)renderer->__shader);
            vao_draw_with_vbo(&renderer->__vao, &vbo);

    vao_unbind();

    vbo_destroy(&vbo);

}


void glrenderer2d_draw_quad(glrenderer2d_t *renderer, const glquad_t quad)
{
    if (renderer == NULL) eprint("renderer argument is null");

    assert(renderer->__shader);

    vao_t *vao = &renderer->__vao;
    vbo_t vbo; 

    vao_bind(vao);

        vbo = vbo_init(quad.vertex);
        ebo_t ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, MAX_QUAD_INDICES_CAPACITY);

        vao_set_attributes(vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position));
        vao_set_attributes(vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));

        if (renderer->__texture != NULL) {
            vao_set_attributes(vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex_t ), offsetof(glvertex_t, texture_coord));
            vao_set_attributes(vao, &vbo, 1, GL_UNSIGNED_INT, false, sizeof(glvertex_t ), offsetof(glvertex_t, texture_id));
            gltexture2d_bind(renderer->__texture, 0);
        }

        glshader_bind((glshader_t *)renderer->__shader);
        vao_draw_with_ebo(vao, &ebo);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);

}



void glrenderer2d_draw_from_batch(glrenderer2d_t *renderer, const glbatch_t *batch) 
{
    if (renderer == NULL) eprint("renderer argument is null");
    if (batch == NULL) eprint("batch argument is null");

    assert(!queue_is_empty(&batch->vertices));
    assert(renderer->__shader);

    u64 vertices_count = 0;
    switch(batch->type)
    {
        case GLBT_gltri_t:
            vertices_count = 3 * batch->vertices.len;
        break;

        case GLBT_glquad_t:
            vertices_count = 6 * batch->vertices.len;
        break;
        case GLBT_glcircle_t:
            vertices_count = MAX_VERTICES_PER_CIRCLE * batch->vertices.len;
        break;

        default: eprint("batch type not accounted for");
    }

    vao_t *vao  = &renderer->__vao;
    vbo_t vbo   = vbo_static_init (
                    batch->vertices.array, 
                    batch->vertices.len * batch->vertices.elem_size, 
                    vertices_count);
    ebo_t ebo;

    vao_bind(vao);

        // Ebo setup
        switch(batch->type)
        {
            case GLBT_gltri_t:
            case GLBT_glcircle_t:
                // skipping ...
            break;

            case GLBT_glquad_t:

                memset(GLOBAL_BATCH_POLY_INDICIES_BUFFER, 0, sizeof(GLOBAL_BATCH_POLY_INDICIES_BUFFER));
                __gen_quad_indices(GLOBAL_BATCH_POLY_INDICIES_BUFFER, batch->vertices.len);
                ebo = ebo_init(&vbo, GLOBAL_BATCH_POLY_INDICIES_BUFFER, vertices_count);

            break;

            default: eprint("batch type not accounted for");
        }

        // Attributes setup
        vao_set_attributes(vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position));
        vao_set_attributes(vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));

        if (renderer->__texture != NULL) {
            vao_set_attributes(vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex_t ), offsetof(glvertex_t, texture_coord));
            vao_set_attributes(vao, &vbo, 1, GL_UNSIGNED_INT, false, sizeof(glvertex_t ), offsetof(glvertex_t, texture_id));
            gltexture2d_bind(renderer->__texture, 0);
        }

        glshader_bind((glshader_t *)renderer->__shader);

        // Draw calls
        switch(batch->type)
        {
            case GLBT_gltri_t:
                vao_draw_with_vbo(vao, &vbo);
            break;

            case GLBT_glquad_t: 
                vao_draw_with_ebo(vao, &ebo);
            break;

            case GLBT_glcircle_t:
                //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                vao_draw_with_vbo(vao, &vbo);
            break;

            default: eprint("batch type not accounted for");
        }

    vao_unbind();


    // Deleting ebo
    switch(batch->type)
    {
        case GLBT_glcircle_t:
        case GLBT_gltri_t:
            // skipping
        break;

        case GLBT_glquad_t: 
            ebo_destroy(&ebo);
        break;


        default: eprint("batch type not accounted for");
    }

    vbo_destroy(&vbo);
}



void glrenderer2d_draw_frame_buffer(glframebuffer_t *fbo, const glquad_t quad)
{
    glrenderer2d_t rd = glrenderer2d_init(&fbo->color_shader, &fbo->color_texture);
    glrenderer2d_draw_quad(&rd, quad);
    glrenderer2d_destroy(&rd);
}

#endif

