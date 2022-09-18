#pragma once
#include "./texture2d.h"

/*=============================================================================
                            - OPENGL FRAMEBUFFER - 
=============================================================================*/

typedef struct glframebuffer_t {

    GLuint         id; // Frame buffer object id
    GLuint         rbo_id; // Render buffer object id
    gltexture2d_t  texture2d; // Color attachment texture

} glframebuffer_t;


glframebuffer_t     glframebuffer_init(const u32 screen_width, const u32 screen_height);

#define             glframebuffer_begin_texture(PFRAMEBUFFER)                     __impl_glframebuffer_begin_scene(PFRAMEBUFFER) 
#define             glframebuffer_end_texture(PFRAMEBUFFER)                       __impl_glframebuffer_end_scene(PFRAMEBUFFER)

void                glframebuffer_destroy(glframebuffer_t *);

/*-----------------------------------------------------------------------------
                            IMPLEMENTATION 
-----------------------------------------------------------------------------*/

#ifndef IGNORE_GL_FRAMEBUFFER_IMPLEMENTATION

#define __framebuffer_bind(pfbo)    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, (pfbo)->id))
#define __framebuffer_unbind(pfbo)  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0))

#define __impl_glframebuffer_begin_scene(pfbo) do {\
        __framebuffer_bind(pfbo);\
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));\
} while(0)

#define __impl_glframebuffer_end_scene(pfbo) do {\
        __framebuffer_unbind(pfbo);\
        GL_CHECK(glDisable(GL_DEPTH_TEST));\
} while(0)


//NOTE: this constructor creates the color attachemnt as the texture and both the depth and stencil buffer as render buffer object

glframebuffer_t glframebuffer_init(const u32 screen_width, const u32 screen_height)
{
    // Initializing the framebuffer
    GLuint fbo_id;
    GL_CHECK(glGenFramebuffers(1, &fbo_id));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_id));

    // Color attachemnt (texture)
    gltexture2d_t texture2d = gltexture2d_empty_init(screen_width, screen_height);
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2d.id, 0));


    // Depth and stencil buffer attachement (render buffer)
    GLuint rbo_id;
    GL_CHECK(glGenRenderbuffers(1, &rbo_id));
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, rbo_id));
    GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screen_width, screen_height));
    GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_id));

    // Checking if the framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        eprint("Error: framebuffer not complete");

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    return (glframebuffer_t ) {
        .id            = fbo_id,
        .rbo_id        = rbo_id,
        .texture2d      = texture2d,
    };
}

void glframebuffer_destroy(glframebuffer_t *fbo)
{
    GL_CHECK(glDeleteFramebuffers(1, &fbo->id));
    gltexture2d_destroy(&fbo->texture2d);
}

#endif 
