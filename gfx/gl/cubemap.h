#pragma once
#include "poglib/gfx/gl/common.h"
#include "poglib/gfx/gl/texture2d.h"
#include "poglib/gfx/gl/types.h"
#include "poglib/gfx/glrenderer3d.h"

typedef struct glcubemap_t {
    u32 glTextureId;
} glcubemap_t;

glcubemap_t glcubemap_init(str_t filepaths[TOTAL_CUBE_FACES]);
void        glcubemap_destroy(glcubemap_t *self);

glcubemap_t glcubemap_init(str_t filepaths[TOTAL_CUBE_FACES])
{
    u32 textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    i32 width, height, nrChannels;
    for (u32 face_index = 0; face_index < TOTAL_CUBE_FACES; face_index++)
    {
        u8 *data = stbi_load(
            filepaths[face_index].data, 
            &width, 
            &height, 
            &nrChannels, 
            0
        );

        if(!data) eprint("failed to load file `%s`", filepaths[face_index].data);

        GL_CHECK(
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_index, 
                0, 
                GL_RGB, 
                width, 
                height, 
                0, 
                GL_RGB, 
                GL_UNSIGNED_BYTE, 
                data)
        );
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return (glcubemap_t) {
        .glTextureId = textureID
    };
}

gltexture2d_t glcubemap_get_texture_handle(glcubemap_t *self)
{
    return (gltexture2d_t) {
        .id = self->glTextureId
    };
}

void glcubemap_destroy(glcubemap_t *self)
{
    GL_CHECK(glDeleteTextures(1, &self->glTextureId)); 
}


glrendercall_t glcubemap_get_render_config(glcubemap_t *self)
{
    return (glrendercall_t) {
        .draw_mode = GL_TRIANGLES,
        .vtx = {
            .size = sizeof(DEFAULT_CUBE_VERTICES_24),
            .data = DEFAULT_CUBE_VERTICES_24,
        },
        .idx = {0},
        .attrs = {
            .count = 1,
            .attr[GL_VTX_ATTRIBUTE_TYPE_COUNT] = {
                [GL_VTX_ATTRIBUTE_TYPE_VTX] = {
                }
            }
        },
        .textures = {
            .count = 1,
            .data = glcubemap_get_texture_handle(self),
        }, 
        .shader_config = {
        }
    }
}
