#pragma once
#include "./texture2d.h"

/*=============================================================================
                            - OPENGL FRAMEBUFFER - 
=============================================================================*/

typedef struct glframebuffer_t {
    u32            id;
    u32            rbo_id;
    gltexture2d_t  texture2d;
    u16            width;
    u16            height;
} glframebuffer_t;

glframebuffer_t     glframebuffer_init(const u16 screen_width, const u16 screen_height);
void                glframebuffer_begin(glframebuffer_t *pfbo);
void                glframebuffer_end(void);
void                glframebuffer_destroy(glframebuffer_t *fbo);

/*-----------------------------------------------------------------------------
                            IMPLEMENTATION 
-----------------------------------------------------------------------------*/

#ifndef IGNORE_GL_FRAMEBUFFER_IMPLEMENTATION

glframebuffer_t glframebuffer_init(const u16 screen_width, const u16 screen_height)
{
    GLuint fbo_id;
    GL_CHECK(glGenFramebuffers(1, &fbo_id));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_id));

    // Color attachment (texture)
    gltexture2d_t texture2d = gltexture2d_empty_init(screen_width, screen_height);
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2d.id, 0));

    // Depth/Stencil attachment (RBO)
    GLuint rbo_id;
    GL_CHECK(glGenRenderbuffers(1, &rbo_id));
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, rbo_id));
    GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screen_width, screen_height));
    GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_id));

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        eprint("Error: framebuffer not complete");

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    return (glframebuffer_t) {
        .id        = fbo_id,
        .rbo_id    = rbo_id,
        .texture2d = texture2d,
        .width     = screen_width,
        .height    = screen_height
    };
}

void glframebuffer_begin(glframebuffer_t *const pfbo)
{
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, pfbo->id));
    GL_CHECK(glViewport(0, 0, pfbo->width, pfbo->height));
    GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT | (pfbo->rbo_id != 0 ? GL_COLOR_BUFFER_BIT : 0)));
}

void glframebuffer_end(void)
{
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void glframebuffer_destroy(glframebuffer_t *const fbo)
{
    GL_CHECK(glDeleteFramebuffers(1, &fbo->id));
    if (fbo->rbo_id != 0) {
        GL_CHECK(glDeleteRenderbuffers(1, &fbo->rbo_id));
    }
    gltexture2d_destroy(&fbo->texture2d);
}

#endif
