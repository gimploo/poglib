#pragma once
#include "poglib/gfx/gl/common.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/gfx/gl/texture2d.h"
#include "poglib/gfx/gl/types.h"

typedef struct glcubemap_t {
    const u32 texture_id;
} glcubemap_t;

glcubemap_t     glcubemap__init(const str_t filepaths[TOTAL_CUBE_FACES]);
void            glcubemap__destroy(glcubemap_t *self);

glcubemap_t glcubemap__init(const str_t filepaths[TOTAL_CUBE_FACES])
{
    u32 textureID;
    GL_CHECK(glGenTextures(1, &textureID));
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

    i32 width, height, nrChannels;
    for (u32 face_index = 0; face_index < TOTAL_CUBE_FACES; face_index++)
    {
        stbi_set_flip_vertically_on_load(false);
        u8 *data = stbi_load(
            filepaths[face_index].data, 
            &width, 
            &height, 
            &nrChannels, 
            4
        );

        if(!data) eprint("failed to load file `%s`", filepaths[face_index].data);

        GL_CHECK(
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_index, 
                0, 
                GL_RGBA, 
                width, 
                height, 
                0, 
                GL_RGBA, 
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
        .texture_id = textureID,
    };
}

void glcubemap_bind(glcubemap_t *self)
{
    GL_CHECK(
        glBindTexture(GL_TEXTURE_CUBE_MAP, self->texture_id)
    );
}

void glcubemap__destroy(glcubemap_t *self)
{
    GL_CHECK(glDeleteTextures(1, &self->texture_id)); 
}


