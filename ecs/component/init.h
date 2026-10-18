#pragma once
#include "../common.h"

void gl_directional_light_shadow_texture_init(ecs_component_directional_light_t *self)
{
    u32 tex_id;
    GL_CHECK(glGenTextures(1, &tex_id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex_id));
    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_DEPTH_COMPONENT24, // The format opengl will store the pixel data in
        SHADOW_WIDTH,
        SHADOW_HEIGHT,
        0,
        GL_DEPTH_COMPONENT, // The format the buf variable is in
        GL_FLOAT,
        0
    ));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));

    /*
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    GL_CHECK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (f32[]){1.0f, 1.0f, 1.0f, 1.0f}));
    */

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    self->internal.shadow_texid = tex_id;
    self->internal.shadowmap_matrix = MATRIX4F_IDENTITY;
}
