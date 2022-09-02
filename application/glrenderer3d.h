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

typedef struct glrenderer3d_t {

    const glshader_t    *shader;
    const gltexture2d_t *texture;

} glrenderer3d_t ;


glrenderer3d_t      glrenderer3d(const glshader_t *, const gltexture2d_t *);
void                glrenderer3d_draw_cube(const glrenderer3d_t *renderer);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION
-----------------------------------------------------------------------------*/

#ifndef IGNORE_GLRENDERER2D_IMPLEMENTATION

//NOTE: make sure to not have texture uniform if your passing NULL as texture argument
glrenderer3d_t glrenderer3d(const glshader_t *shader, const gltexture2d_t *texture)
{
    return (glrenderer3d_t ) {
        .shader = shader,
        .texture = texture,
    };

}

void glrenderer3d_draw_cube(const glrenderer3d_t *renderer)
{    
    if (renderer == NULL) eprint("renderer argument is null");
    assert(renderer->shader);

    const f32 CUBE_VERTICES[] = {
        // front
        -1.0, -1.0,  1.0,
         1.0, -1.0,  1.0,
         1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
        // back
        -1.0, -1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0
    };

    vao_t vao = vao_init();
    vbo_t vbo; 

    vao_bind(&vao);
            const u32 count = MAX_VERTICES_PER_CUBE + MAX_UVS_PER_CUBE;
            vbo = vbo_static_init(&CUBE_VERTICES, sizeof(CUBE_VERTICES), count);
            ebo_t ebo = ebo_init(&vbo, DEFAULT_CUBE_INDICES, ARRAY_LEN(DEFAULT_CUBE_INDICES));
            vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, 3 * sizeof(f32), 0);
            /*vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));*/

            if (renderer->texture != NULL) {
                /*vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, false, 2 * sizeof(f32), offsetof(glcube_t, uvs));*/
                /*vao_set_attributes(&vao, &vbo,1, GL_UNSIGNED_INT, false, sizeof(glvertex_t), offsetof(glvertex_t, texture_id));*/
                gltexture2d_bind(renderer->texture, 0);
            }

            glshader_bind((glshader_t *)renderer->shader);
            vao_draw_with_ebo(&vao, &ebo);

    vao_unbind();

    vbo_destroy(&vbo);
    vao_destroy(&vao);

}


#endif

