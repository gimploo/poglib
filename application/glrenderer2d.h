#pragma once
#include <GL/glew.h>
#include <poglib/math/shapes.h>
#include <poglib/application/gfx/gl/shader.h>
#include <poglib/application/gfx/gl/texture2d.h>
#include <poglib/application/gfx/gl/globjects.h>
#include <poglib/application/gfx/gl/glframebuffer.h>
#include <poglib/application/gfx/gl/types.h>

/*=============================================================================
                        - OPENGL 2D RENDERER -
=============================================================================*/

typedef struct glrenderer2d_t {

    const glshader_t    *shader;
    const gltexture2d_t *texture;

} glrenderer2d_t ;


glrenderer2d_t      glrenderer2d(const glshader_t *shader, const gltexture2d_t *texture);

void                glrenderer2d_draw_quad(const glrenderer2d_t *renderer, const glquad_t quad);
void                glrenderer2d_draw_triangle(const glrenderer2d_t *renderer, const gltri_t tri);
void                glrenderer2d_draw_circle(const glrenderer2d_t *renderer, const glcircle_t circle);

void                glrenderer2d_draw_from_batch(const glrenderer2d_t *renderer, const glbatch_t *batch);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION
-----------------------------------------------------------------------------*/

#ifndef IGNORE_GLRENDERER2D_IMPLEMENTATION

#define MAX_QUAD_CAPACITY_PER_DRAW_CALL 10000

global u32 GLOBAL_BATCH_POLY_INDICIES_BUFFER[MAX_QUAD_CAPACITY_PER_DRAW_CALL * MAX_VERTICES_PER_CIRCLE];

global u32 GLOBAL_POLY_INDICIES_BUFFER[MAX_VERTICES_PER_CIRCLE];



//NOTE: make sure to not have texture uniform if your passing NULL as texture argument
glrenderer2d_t glrenderer2d(const glshader_t *shader, const gltexture2d_t *texture)
{
    return (glrenderer2d_t ) {
        .shader = shader,
        .texture = texture,
    };

}

void glrenderer2d_draw_triangle(const glrenderer2d_t *renderer, const gltri_t tri)
{    
    if (renderer == NULL) eprint("renderer argument is null");
    assert(renderer->shader);

    vao_t vao = vao_init();
    vbo_t vbo; 

    vao_bind(&vao);

            vbo = vbo_init(tri.vertex);
            vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position));
            vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));

            if (renderer->texture != NULL) {
                vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_coord));
                vao_set_attributes(&vao, &vbo,1, GL_UNSIGNED_INT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_id));
                gltexture2d_bind(renderer->texture, 0);
            }

            glshader_bind((glshader_t *)renderer->shader);
            vao_draw_with_vbo(&vao, &vbo);

    vao_unbind();

    vbo_destroy(&vbo);
    vao_destroy(&vao);

}

void glrenderer2d_draw_circle(const glrenderer2d_t *renderer, const glcircle_t circle)
{    
    if (renderer == NULL) eprint("renderer argument is null");
    assert(renderer->shader);

    vao_t vao = vao_init();
    vbo_t vbo; 

    vao_bind(&vao);

            vbo = vbo_init(circle.vertex);
            vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position));
            vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));

            if (renderer->texture != NULL) {
                vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_coord));
                vao_set_attributes(&vao, &vbo,1, GL_UNSIGNED_INT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_id));
                gltexture2d_bind(renderer->texture, 0);
            }

            glshader_bind((glshader_t *)renderer->shader);
            vao_draw_with_vbo(&vao, &vbo);

    vao_unbind();

    vbo_destroy(&vbo);
    vao_destroy(&vao);

}


void glrenderer2d_draw_quad(const glrenderer2d_t *renderer, const glquad_t quad)
{
    if (renderer == NULL) eprint("renderer argument is null");

    assert(renderer->shader);

    vao_t vao = vao_init();
    vbo_t vbo; 

    vao_bind(&vao);

        vbo = vbo_init(quad.vertex);
        ebo_t ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, MAX_QUAD_INDICES_CAPACITY);

        vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position));
        vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));

        if (renderer->texture != NULL) {
            vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex_t ), offsetof(glvertex_t, texture_coord));
            vao_set_attributes(&vao, &vbo, 1, GL_UNSIGNED_INT, false, sizeof(glvertex_t ), offsetof(glvertex_t, texture_id));
            gltexture2d_bind(renderer->texture, 0);
        }

        glshader_bind((glshader_t *)renderer->shader);
        vao_draw_with_ebo(&vao, &ebo);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
    vao_destroy(&vao);

}



void glrenderer2d_draw_from_batch(const glrenderer2d_t *renderer, const glbatch_t *batch) 
{
    if (renderer == NULL) eprint("renderer argument is null");
    if (batch == NULL) eprint("batch argument is null");
    if(glbatch_is_empty(batch)) eprint("batch queue is empty");

    assert(renderer->shader);

    u64 vertices_count = 0;
    switch(batch->type)
    {
        case GLBT_gltri_t:
            vertices_count = 3 * batch->globjs.len;
        break;

        case GLBT_glquad_t:
            vertices_count = 6 * batch->globjs.len;
        break;

        case GLBT_glcircle_t:
            vertices_count = MAX_VERTICES_PER_CIRCLE * batch->globjs.len;
        break;

        default: eprint("batch type not accounted for");
    }

    vao_t vao  = vao_init();
    vbo_t vbo   = vbo_static_init(
                    batch->globjs.__array, 
                    batch->globjs.len * batch->globjs.__elem_size, 
                    vertices_count);
    ebo_t ebo;

    vao_bind(&vao);

        // Ebo setup
        switch(batch->type)
        {
            case GLBT_gltri_t:
            case GLBT_glcircle_t:
                // skipping ...
            break;

            case GLBT_glquad_t:

                memset(GLOBAL_BATCH_POLY_INDICIES_BUFFER, 0, sizeof(GLOBAL_BATCH_POLY_INDICIES_BUFFER));
                __gen_quad_indices(GLOBAL_BATCH_POLY_INDICIES_BUFFER, batch->globjs.len);
                ebo = ebo_init(&vbo, GLOBAL_BATCH_POLY_INDICIES_BUFFER, vertices_count);

            break;

            default: eprint("batch type not accounted for");
        }

        // Attributes setup
        vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position));
        vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));

        if (renderer->texture != NULL) {
            vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex_t ), offsetof(glvertex_t, texture_coord));
            vao_set_attributes(&vao, &vbo, 1, GL_UNSIGNED_INT, false, sizeof(glvertex_t ), offsetof(glvertex_t, texture_id));
            gltexture2d_bind(renderer->texture, 0);
        }

        glshader_bind((glshader_t *)renderer->shader);

        // Draw calls
        switch(batch->type)
        {
            case GLBT_gltri_t:
                vao_draw_with_vbo(&vao, &vbo);
            break;

            case GLBT_glquad_t: 
                vao_draw_with_ebo(&vao, &ebo);
            break;

            case GLBT_glcircle_t:
                //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                vao_draw_with_vbo(&vao, &vbo);
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
    vao_destroy(&vao);
}




#endif

