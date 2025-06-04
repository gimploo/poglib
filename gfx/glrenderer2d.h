#pragma once
#include <GL/glew.h>
#include "gl/shader.h"
#include "gl/texture2d.h"
#include "gl/objects.h"
#include "gl/framebuffer.h"
#include "gl/types.h"

/*=============================================================================
                        - OPENGL 2D RENDERER -
=============================================================================*/

#define MAX_TEXTURES_ALLOWED_PER_RENDER 10

typedef struct glrenderer2d_t {

    const glshader_t    *shader;
    const gltexture2d_t *texture;

} glrenderer2d_t ;


void                glrenderer2d_draw_quad(const glrenderer2d_t *renderer, const glquad_t quad);
void                glrenderer2d_draw_triangle(const glrenderer2d_t *renderer, const gltri_t tri);
void                glrenderer2d_draw_circle(const glrenderer2d_t *renderer, const glcircle_t circle);
void                glrenderer2d_draw_polygon(const glrenderer2d_t *renderer, const glpolygon_t polygon);
void                glrenderer2d_draw_from_batch(const glrenderer2d_t *renderer, const glbatch_t *batch);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION
-----------------------------------------------------------------------------*/

#ifndef IGNORE_GLRENDERER2D_IMPLEMENTATION

#define MAX_QUAD_CAPACITY_PER_DRAW_CALL 10000

global u32 GLOBAL_BATCH_POLY_INDICIES_BUFFER[MAX_QUAD_CAPACITY_PER_DRAW_CALL * MAX_VERTICES_PER_CIRCLE];

global u32 GLOBAL_POLY_INDICIES_BUFFER[MAX_VERTICES_PER_CIRCLE];



void glrenderer2d_draw_triangle(const glrenderer2d_t *renderer, const gltri_t tri)
{    
    if (renderer == NULL) eprint("renderer argument is null");
    assert(renderer->shader);

    vao_t vao = vao_init();
    vbo_t vbo; 

    vao_bind(&vao);

            vbo = vbo_static_init(tri.vertex, sizeof(tri.vertex), sizeof(tri.vertex) / sizeof(glvertex2d_t ));
            vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, position));
            vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, color));

            if (renderer->texture != NULL) {
                vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, uv));
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

            vbo = vbo_static_init(circle.vertex, sizeof(circle.vertex), sizeof(circle.vertex) / sizeof(glvertex2d_t ));
            vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, position));
            vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, color));

            if (renderer->texture != NULL) {
                vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, uv));
                gltexture2d_bind(renderer->texture, 0);
            }

            glshader_bind((glshader_t *)renderer->shader);
            vao_draw_with_vbo_in_mode(&vao, &vbo, GL_TRIANGLE_FAN);

    vao_unbind();

    vbo_destroy(&vbo);
    vao_destroy(&vao);

}

void glrenderer2d_draw_polygon(const glrenderer2d_t *renderer, const glpolygon_t polygon)
{    
    if (renderer == NULL) eprint("renderer argument is null");
    assert(renderer->shader);

    vao_t vao = vao_init();
    vbo_t vbo; 

    vao_bind(&vao);

            u64 vsize = polygon.sides * 3 * sizeof(glvertex2d_t );
            u64 vertex_count = polygon.sides * 3;
            vbo = vbo_static_init(
                 polygon.vertices.vertex , 
                 vsize, vertex_count);
            vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, position));
            vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, color));

            if (renderer->texture != NULL) {
                vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, uv));
                gltexture2d_bind(renderer->texture, 0);
            }

            glshader_bind((glshader_t *)renderer->shader);
            vao_draw_with_vbo_in_mode(&vao, &vbo, GL_TRIANGLE_FAN);

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

        vbo = vbo_static_init(quad.vertex, sizeof(quad.vertex), sizeof(quad.vertex) / sizeof(glvertex2d_t ));
        ebo_t ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, MAX_QUAD_INDICES_CAPACITY);

        vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, position));
        vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, color));

        if (renderer->texture != NULL) {
            vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex2d_t ), offsetof(glvertex2d_t, uv));
            gltexture2d_bind(renderer->texture, 0);
        }

        glshader_bind((glshader_t *)renderer->shader);
        vao_draw_with_ebo(&vao, &ebo);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
    vao_destroy(&vao);

}



//FIXME: this can be optimized further using GL Instanced apis
void glrenderer2d_draw_from_batch(const glrenderer2d_t *renderer, const glbatch_t *batch) 
{
    if (renderer == NULL) eprint("renderer argument is null");
    if (batch == NULL) eprint("batch argument is null");
    if(glbatch_is_empty(batch)) eprint("batch queue is empty");

    assert(renderer->shader);

    const u64 vertices_count = batch->__meta.nvertex * batch->globjs.len;
    vao_t vao  = vao_init();
    vbo_t vbo   = vbo_static_init(
                    batch->globjs.__data, 
                    batch->globjs.len * batch->globjs.__elem_size, 
                    vertices_count);
    ebo_t ebo;

    vao_bind(&vao);

        // Ebo setup
        switch(batch->__meta.type)
        {
            case GLBT_gltri_t:
            case GLBT_glpolygon_t:
            case GLBT_glcircle_t:
                eprint("batch type not accounted for");
            break;

            case GLBT_glquad_t:

                memset(GLOBAL_BATCH_POLY_INDICIES_BUFFER, 0, sizeof(GLOBAL_BATCH_POLY_INDICIES_BUFFER));
                __gen_quad_indices(GLOBAL_BATCH_POLY_INDICIES_BUFFER, batch->globjs.len);
                ebo = ebo_init(&vbo, GLOBAL_BATCH_POLY_INDICIES_BUFFER, vertices_count);

            break;

            default: eprint("batch type not accounted for");
        }

        // Attributes setup
        vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, position));
        vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex2d_t), offsetof(glvertex2d_t, color));

        glshader_bind((glshader_t *)renderer->shader);

        if (renderer->texture != NULL) {

            vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, sizeof(glvertex2d_t ), offsetof(glvertex2d_t, uv));

            gltexture2d_bind(renderer->texture, 0);
        }


        // Draw calls
        switch(batch->__meta.type)
        {
            case GLBT_gltri_t:
                vao_draw_with_vbo(&vao, &vbo);
            break;

            case GLBT_glquad_t: 
                vao_draw_with_ebo(&vao, &ebo);
            break;

            case GLBT_glcircle_t:
            case GLBT_glpolygon_t:
                vao_draw_with_vbo_in_mode(&vao, &vbo, GL_TRIANGLE_FAN);
            break;

            default: eprint("batch type not accounted for");
        }

    vao_unbind();


    // Deleting ebo
    switch(batch->__meta.type)
    {
        case GLBT_glcircle_t:
        case GLBT_glpolygon_t:
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

