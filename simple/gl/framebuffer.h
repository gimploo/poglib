#ifndef __MY_GL_FRAME_BUFFER_H__
#define __MY_GL_FRAME_BUFFER_H__

#include "./_common.h"

/*====================================================================================
 // OpenGL Framebuffer 
====================================================================================*/

typedef struct gl_framebuffer_t {

    GLuint id;
    GLuint color_texture_id;  
    GLuint render_buffer_id;

    u32 width, height;

} gl_framebuffer_t;

/*------------------------------------------------------------------------------
 // Declaration
------------------------------------------------------------------------------*/

gl_framebuffer_t    gl_framebuffer_init(u32 count, u32 width, u32 height);
void                gl_framebuffer_attach_color_texture(gl_framebuffer_t *fbo); //NOTE: use this function if your planning to use a shader with this texture
void                gl_framebuffer_attach_render_buffer(gl_framebuffer_t *fbo); //NOTE: faster and isnt accessable from a shader

//NOTE:(macro)      gl_framebuffer_store_begin_scene(framebuffer_t *fbo) -> void
//NOTE:(macro)      gl_framebuffer_store_end_scene(framebuffer_t *fbo) -> void
//


/*------------------------------------------------------------------------------
 // Implementation
------------------------------------------------------------------------------*/

gl_framebuffer_t gl_framebuffer_init(u32 count, u32 width, u32 height)
{
    gl_framebuffer_t fbo;
    fbo.width = width;
    fbo.height = height;
    GL_CHECK(glGenFramebuffers(count, &fbo.id));
    return fbo;
}

#define __gl_framebuffer_bind(pfbo)         GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, (pfbo)->id))
#define __gl_framebuffer_unbind(pfbo)       GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0 ))

void gl_framebuffer_attach_color_texture(gl_framebuffer_t *fbo)
{
    __gl_framebuffer_bind(fbo);

        GL_CHECK(glGenTextures(1, &fbo->color_texture_id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, fbo->color_texture_id));
        GL_CHECK(glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, fbo->width, fbo->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));	
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->color_texture_id, 0));

    __gl_framebuffer_unbind(fbo);
}


void gl_framebuffer_attach_render_buffer(gl_framebuffer_t *fbo)
{
    __gl_framebuffer_bind(fbo);
        
        GL_CHECK(glGenRenderbuffers(1, &fbo->render_buffer_id));
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, fbo->render_buffer_id));
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbo->width, fbo->height));
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo->render_buffer_id));

    __gl_framebuffer_unbind(fbo);
}


#define gl_framebuffer_store_begin_scene(pfbo) do {\
    __gl_framebuffer_bind(pfbo);\
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);\
    glEnable(GL_DEPTH_TEST);\
} while(0)

#define gl_framebuffer_store_end_scene(pfbo) do {\
    __gl_framebuffer_unbind(pfbo);\
    glDisable(GL_DEPTH_TEST);\
} while(0)
    

#endif //__MY_GL_FRAME_BUFFER_H__
