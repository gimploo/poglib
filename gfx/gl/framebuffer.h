#pragma once
#include "./common.h"

typedef struct glframebuffer_t {
    u32 id;
} glframebuffer_t;

#ifndef IGNORE_GL_FRAMEBUFFER_IMPLEMENTATION

glframebuffer_t glframebuffer_depthonly_init(void)
{
    u32 fbo_id;
    GL_CHECK(glGenFramebuffers(1, &fbo_id));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_id));

    GL_CHECK(glDrawBuffer(GL_NONE));
    GL_CHECK(glReadBuffer(GL_NONE));

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    return (glframebuffer_t) {
        .id = fbo_id,
    };
}


#endif
