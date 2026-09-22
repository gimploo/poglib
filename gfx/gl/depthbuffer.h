#pragma once
#include "./texture2d.h"
#include "poglib/gfx/gl/cubemap.h"

//-- Shadow: Depth buffer ---------------

/*
//NOTE: HOW TO 

    DURING RENDER LOOP (Pass 1 - Shadow Map Pass)
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    for (int i = 0; i < num_lights; ++i) 
    {
        // Dynamically attach Layer 'i' of the texture array to the FBO
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowArrayTex, 0, i);

        glClear(GL_DEPTH_BUFFER_BIT);
        render_scene_from_light_perspective(i);
    }

*/

typedef struct {
    GLuint         fbo_id;
    u32            tex_id;
    u32            width;
    u32            height;
} gldepthbuffer_t;


gldepthbuffer_t     gldepthbuffer_texturearray_init(const u8 capacity, const u16 map_width, const u16 map_height);
gldepthbuffer_t     gldepthbuffer_cubemap_init(const u16 map_width, const u16 map_height);
void                gldepthbuffer_destroy(gldepthbuffer_t *const self);


gldepthbuffer_t gldepthbuffer_texturearray_init(const u8 capacity, const u16 map_width, const u16 map_height)
{
    GLuint fbo_id;
    GL_CHECK(glGenFramebuffers(1, &fbo_id));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_id));

    // Create 32-bit Floating Point Depth Texture
    GLuint depth_array_id;
    GL_CHECK(glGenTextures(1, &depth_array_id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, depth_array_id));
    GL_CHECK(glTexImage3D(
        GL_TEXTURE_2D_ARRAY, 
        0, 
        GL_DEPTH_COMPONENT32F, 
        (i32)map_width, 
        (i32)map_height, 
        capacity,
        0, 
        GL_DEPTH_COMPONENT, 
        GL_FLOAT, 
        NULL
    ));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, (f32 []){ 1.0f, 1.0f, 1.0f, 1.0f }));

    GL_CHECK(glDrawBuffer(GL_NONE));
    GL_CHECK(glReadBuffer(GL_NONE));

    GL_CHECK(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_array_id, 0, 0));
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        eprint("Error: shadow framebuffer not complete");

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    return (gldepthbuffer_t) {
        .fbo_id    = fbo_id,
        .width     = map_width,
        .height    = map_height,
        .tex_id    = depth_array_id,
    };
}

void gldepthbuffer_destroy(gldepthbuffer_t *const self)
{
    GL_CHECK(glDeleteFramebuffers(1, &self->fbo_id));
    GL_CHECK(glDeleteTextures(1, &self->tex_id));
}

gldepthbuffer_t gldepthbuffer_cubemap_init(const u16 map_width, const u16 map_height)
{
    GLuint fbo_id;
    GL_CHECK(glGenFramebuffers(1, &fbo_id));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_id));

    GLuint depthCubemap;
    GL_CHECK(glGenTextures(1, &depthCubemap));
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap));

    for (u8 index = 0; index < 6; ++index)
    {
        GL_CHECK(glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 
            0, 
            GL_DEPTH_COMPONENT32F, 
            map_width, map_height, 
            0, 
            GL_DEPTH_COMPONENT, 
            GL_FLOAT, 
            NULL)
        );
    }

    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0));

    GL_CHECK(glDrawBuffer(GL_NONE));
    GL_CHECK(glReadBuffer(GL_NONE));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        eprint("Error: Cubemap shadow framebuffer is incomplete!");
    }

    // Reset framebuffer state binding
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    return (gldepthbuffer_t) {
        .fbo_id    = fbo_id,
        .width     = map_width,
        .height    = map_height,
        .tex_id    = depthCubemap,
    };
}

