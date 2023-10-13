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
        int             top;
    } textures;

} glrenderer3d_t ;

typedef struct {

    //NOTE: we are in the assumption that only float values are allowed via attributes
    //other types are not supported 

    const u8 nattr;
    const struct {

        u8 ncmp;
        u8 buffer_index;
        struct {
            u32 offset;
            u32 stride;
        } interleaved;

    } attr[10];

    const u8 nbuffer;
    const struct {

        u32 size;
        u8 *data;

        struct {

            u32 nmemb;
            u8 *data;

        } indexbuffer;

    } buffer[10];

    const glshader_t *shader;

    const u8 ntexture;
    const struct {

        gltexture2d_t *data;

    } texture[10];

} glrendererconfig_t;



void                glrenderer3d_draw_cube(const glrenderer3d_t *renderer);
void                glrenderer3d_draw_mesh(const glrenderer3d_t *, const glmesh_t *);
void                glrenderer3d_draw_model(const glrenderer3d_t *, const glmodel_t *);
void                glrenderer3d_draw(const glrendererconfig_t config);


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
    if (self->textures.data && self->textures.top > 0)
        for (int i = 0; i < self->textures.top; i++)
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
    if (self->textures.data && self->textures.top > 0)
        for (int i = 0; i < self->textures.top; i++)
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


//NOTE: we have all the buffers combined to one single vbo and this creates a concern
//of whether can the vbo be big enough to store all of it - needs more testing
void glrenderer3d_draw(const glrendererconfig_t config)
{
    ASSERT(config.nbuffer <= 10);
    ASSERT(config.nattr <= 10 && config.nattr > 0);
    ASSERT(config.shader);

    // this to store all the buffer offsets for attributes to reference from
    // also be thoughtfull that this buffer can only be referenced via buffer index and 
    // nothing else as it can have empty values if multiple buffers with index buffers are
    // present
    u32 mega_vbo_buffoffsets[10] = {0};
    u32 mega_vbo_size = 0;

    struct {
        vao_t data[5];
        i32 top;
    } vaos = {{0}, -1};

    struct {
        vbo_t data[5];
        i32 top;
    } vbos = {{0}, -1};

    struct {
        ebo_t data[5];
        i32 top;
    } ebos = {{0}, -1};


    // setting the vbos and ebos for those buffers that have them
    for (u8 i = 0; i < config.nbuffer; i++) 
    {
        if (config.buffer[i].indexbuffer.data) {

            ASSERT(vbos.top < ARRAY_LEN(vbos.data));
            vbos.data[++vbos.top] = vbo_static_init(
                                config.buffer[i].data, 
                                config.buffer[i].size, 
                                config.buffer[i].indexbuffer.nmemb);
            vbos.buffer_idx[vbos.top] = i;

            ebos.data[++ebos.top] = ebo_init(
                                &vbos.data[vbos.top], 
                                (u32 *)config.buffer[i].indexbuffer.data, 
                                config.buffer[i].indexbuffer.nmemb);
            ebos.vbos[ebos.top] = &vbos.data[vbos.top];
            vbos.ebos[vbos.top] = &ebos.data[ebos.top];

            ebos.vaos[ebos.top] = vao_init();

            continue;
        }
        mega_vbo_buffoffsets[i] = mega_vbo_size;
        mega_vbo_size += config.buffer[i].size;
    }

    vao_t vao = vao_init();

    // binding the mega vbo
    if (mega_vbo_size != 0) {

        vao_bind(&vao);

        vbos.data[0] = vbo_static_init(NULL, mega_vbo_size, 0);
        vbo_bind(&vbos.data[0]);

        // setting the mega vbo buffer
        for (int i = 0, offset = 0; i < config.nbuffer; i++) 
        {
            if (config.buffer[i].indexbuffer.data) continue;

            GL_CHECK(glBufferSubData(
                    GL_ARRAY_BUFFER, 
                    offset, 
                    config.buffer[i].size, 
                    config.buffer[i].data));

            offset += config.buffer[i].size;
        }
        vao_unbind();
    }


    //setup attributes
    u8 ncmps = 0;
    for (u32 i = 0; i < config.nattr; i++)
    {
        ASSERT(config.attr[i].buffer_index >= 0); 
        ASSERT(config.attr[i].buffer_index < config.nbuffer); 


        if (config.buffer[config.attr[i].buffer_index].indexbuffer.data) {
            u8 vbo_index = 0;

            for (u8 index = 1; index <= vbos.top; index++)
                if (vbos.buffer_idx[index] == config.attr[i].buffer_index) {
                    vbo_index = index;
                    break;
                }

            vao_bind(vbos.ebos->vaos[vbo_index]);
            ebo_bind(vbos.ebos[vbo_index]);
            vbo_bind(&vbos.data[vbo_index]);
            vao_set_attributes(
                &vao, 
                &vbos.data[vbo_index], 
                config.attr[i].ncmp, 
                GL_FLOAT, 
                false, 
                config.attr[i].interleaved.stride, 
                config.attr[i].interleaved.offset);

            continue;
        }
        vbo_bind(&vbos.data[0]);
        vao_set_attributes(
            &vao, 
            &vbos.data[0], 
            config.attr[i].ncmp, 
            GL_FLOAT, 
            false, 
            config.attr[i].interleaved.stride, 
            mega_vbo_buffoffsets[config.attr[i].buffer_index] + config.attr[i].interleaved.offset);

        ncmps += config.attr[i].ncmp;
    }


    glshader_bind(config.shader);
    for (int i = 0; i < config.ntexture; i++)
        gltexture2d_bind(config.texture[i].data, i);


    if (mega_vbo_size != 0) {
        vbos.data[0].vertex_count = mega_vbo_size / (sizeof(f32) * ncmps);
        vbo_bind(&vbos.data[0]);
        vao_draw_with_vbo(&vao, &vbos.data[0]);
    }

    for (u8 ebo_idx = 0; ebo_idx <= ebos.top; ebo_idx++) {
        vbo_bind(ebos.vbos[ebo_idx]);
        ebo_bind(&ebos.data[ebo_idx]);
        vao_draw_with_ebo(&vao, &ebos.data[ebo_idx]);
    }

    vbo_unbind();
    vao_unbind();

    for (u8 ebo_idx = 0; ebo_idx <= ebos.top; ebo_idx++)
        ebo_destroy(&ebos.data[ebo_idx]);

    for (u8 vbo_idx = 0; vbo_idx <= vbos.top; vbo_idx++)
        vbo_destroy(&vbos.data[vbo_idx]);

    vao_destroy(&vao);
}
#endif

