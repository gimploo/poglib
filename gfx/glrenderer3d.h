#pragma once
#include <GL/glew.h>
#include "gl/shader.h"
#include "gl/texture2d.h"
#include "gl/objects.h"
#include "gl/framebuffer.h"
#include "gl/types.h"
#include "model/assimp.h"

/*=============================================================================
                        - OPENGL 2D RENDERER -
=============================================================================*/

typedef struct glrenderer3d_t {

    const glshader_t    *shader;
    struct {
        gltexture2d_t   *data;
        int             count;
    } textures;

} glrenderer3d_t ;

typedef struct {

    //NOTE: we are in the assumption that only float values are allowed via attributes
    //other types are not supported 
    const struct {
        u8 ncmp;
    } attr[10];
    const u8 nattr;

    const struct {
        u32 size;
        u8 *data;
    } subbuffer[10];
    const u8 nsubbuffer;

    const struct {
        u32 nmemb;
        u8 *data;
    } index;

    const glshader_t *shader;

    const struct {
        gltexture2d_t *data;
    } texture[10];
    const u8 ntexture;

} glrendererconfig_t;



void                glrenderer3d_draw_cube(const glrenderer3d_t *renderer);
void                glrenderer3d_draw_mesh(const glrenderer3d_t *, const glmesh_t *);
void                glrenderer3d_draw_model(const glrenderer3d_t *, const glmodel_t *);
void                glrenderer3d_draw_mesh_custom(const glrendererconfig_t config);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION
-----------------------------------------------------------------------------*/

#ifndef IGNORE_GLRENDERER2D_IMPLEMENTATION

void glrenderer3d_draw_cube(const glrenderer3d_t *self)
{
    const f32 vertices[] = {
        // vertex             // uv        // normals
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,   
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,   
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f, 
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f, 
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f
    };

    unsigned int VBO, VAO;
    GL_CHECK(glGenVertexArrays(1, &VAO));
    GL_CHECK(glGenBuffers(1, &VBO));
    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))));
    GL_CHECK(glEnableVertexAttribArray(2));

    glshader_bind(self->shader);
    if (self->textures.data && self->textures.count > 0)
        for (int i = 0; i < self->textures.count; i++)
            gltexture2d_bind(&self->textures.data[i], i);

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));

    GL_CHECK(glDeleteVertexArrays(1, &VAO));
    GL_CHECK(glDeleteBuffers(1, &VBO));
}

void glrenderer3d_draw_mesh(const glrenderer3d_t *self, const glmesh_t *mesh)
{
    const slot_t *vtx = &mesh->vtx;
    const slot_t *idx = &mesh->idx;

    vao_t vao = vao_init();
    vao_bind(&vao);

    vbo_t vbo = vbo_static_init(
            vtx->__data, 
            vtx->__elem_size * vtx->len, 
            vtx->len);
    vbo_bind(&vbo);

    ebo_t ebo = ebo_init(&vbo, (const u32 *)idx->__data, idx->len);
    ebo_bind(&ebo);

    //positions
    vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(vec4f_t ), 0);
    //...

    glshader_bind(self->shader);
    if (self->textures.data && self->textures.count > 0)
        for (int i = 0; i < self->textures.count; i++)
            gltexture2d_bind(&self->textures.data[i], i);

    vao_draw_with_ebo(&vao, &ebo);

    vbo_unbind();
    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
    vao_destroy(&vao);
}

void glrenderer3d_draw_model(const glrenderer3d_t *self, const glmodel_t *model)
{
    list_iterator(&model->meshes, mesh) 
        glrenderer3d_draw_mesh(self, (glmesh_t *)mesh);
}


void glrenderer3d_draw_mesh_custom(const glrendererconfig_t config)
{
    ASSERT(config.nsubbuffer <= 10);
    ASSERT(config.nattr <= 10 && config.nattr > 0);
    ASSERT(config.shader);

    vao_t vao = vao_init();
    vao_bind(&vao);

    u32 total_vbo_size = 0;
    for (int i = 0; i < config.nsubbuffer; i++)
        total_vbo_size += config.subbuffer[i].size;

    vbo_t vbo = vbo_static_init(NULL, total_vbo_size, 0);
    vbo_bind(&vbo);

    for (int i = 0, offset = 0; i < config.nsubbuffer; i++) 
    {
        GL_CHECK(glBufferSubData(
                GL_ARRAY_BUFFER, 
                offset, 
                config.subbuffer[i].size, 
                config.subbuffer[i].data));
        offset += config.subbuffer[i].size;
    }

    ebo_t ebo = ebo_init(&vbo, config.index.data, config.index.nmemb);
    ebo_bind(&ebo);

    for (int i = 0, offset = 0; i < config.nattr; i++)
    {
        vao_set_attributes(
                &vao, 
                &vbo, 
                config.attr[i].ncmp, 
                GL_FLOAT, 
                false, 
                0, 
                offset);
        offset += config.subbuffer[i].size;
    }

    glshader_bind(config.shader);
    for (int i = 0; i < config.ntexture; i++)
        gltexture2d_bind(config.texture[i].data, i);

    vao_draw_with_ebo(&vao, &ebo);

    vbo_unbind();
    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
    vao_destroy(&vao);
}
#endif

