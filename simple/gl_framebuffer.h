#ifndef __MY_GL_FRAMEBUFFER_H__
#define __MY_GL_FRAMEBUFFER_H__

#include "./gl_renderer2d.h"

/*====================================================================================
 // OpenGL Framebuffer 
====================================================================================*/

typedef struct gl_framebuffer_t {

    GLuint          frame_buffer_id;
    GLuint          render_buffer_id;
    u32             screen_width; 
    u32             screen_height;
    
    gl_shader_t     color_buffer_shader;
    gl_texture2d_t  color_buffer_texture;


} gl_framebuffer_t;

/*------------------------------------------------------------------------------
 // Declaration
------------------------------------------------------------------------------*/

gl_framebuffer_t    gl_framebuffer_init(u32 window_width, u32 window_height);
void                gl_framebuffer_attach_color_buffer_texture(gl_framebuffer_t *fbo, gl_shader_t shader, const char *unifrom); //NOTE: use this function if your planning to use a shader with this texture
void                gl_framebuffer_attach_render_buffer(gl_framebuffer_t *fbo); //NOTE: faster and isnt accessable from a shader

//NOTE:(macro)      gl_framebuffer_begin_scene(framebuffer_t *fbo) -> void
//NOTE:(macro)      gl_framebuffer_end_scene(framebuffer_t *fbo) -> void

void                gl_framebuffer_draw_quad(gl_framebuffer_t *fbo, gl_quad_t quad);
void                gl_framebuffer_destroy(gl_framebuffer_t *fbo);

/*------------------------------------------------------------------------------
 // Implementation
------------------------------------------------------------------------------*/

#define __gl_framebuffer_bind(pfbo)                         GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (pfbo)->frame_buffer_id))
#define __gl_framebuffer_unbind(pfbo)                       GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 ))
#define __gl_framebuffer_bind_color_buffer_texture(pfbo)    GL_CHECK(glBindTexture(GL_TEXTURE_2D, (pfbo)->color_buffer_texture.id))

#define gl_framebuffer_begin_scene(pfbo) do {\
    __gl_framebuffer_bind(pfbo);\
    glEnable(GL_DEPTH_TEST);\
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);\
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);\
} while(0)

#define gl_framebuffer_end_scene(pfbo) do {\
    __gl_framebuffer_unbind(pfbo);\
    glDisable(GL_DEPTH_TEST);\
} while(0)
    
gl_framebuffer_t gl_framebuffer_init(u32 window_width, u32 window_height)
{
    gl_framebuffer_t fbo;
    fbo.screen_width = window_width;
    fbo.screen_height = window_height;
    fbo.color_buffer_texture = (gl_texture2d_t ){0};
    GL_CHECK(glGenFramebuffers(1, &fbo.frame_buffer_id));
    return fbo;
}

void gl_framebuffer_attach_color_buffer_texture(gl_framebuffer_t *fbo, gl_shader_t shader, const char *uniform_name)
{
    assert(uniform_name);

    // Shader
    fbo->color_buffer_shader = shader;
    int value = 0;
    gl_shader_set_uniform(&fbo->color_buffer_shader, uniform_name, &value, sizeof(int), UT_INT);

    //Texture
    __gl_framebuffer_bind(fbo);

        GL_CHECK(glGenTextures(1, &fbo->color_buffer_texture.id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, fbo->color_buffer_texture.id));
        GL_CHECK(glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, fbo->screen_width, fbo->screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));	
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->color_buffer_texture.id, 0));

    __gl_framebuffer_unbind(fbo);

}

//NOTE: stencil and depth buffers are attached to the render buffer by default
void gl_framebuffer_attach_render_buffer(gl_framebuffer_t *fbo)
{
    __gl_framebuffer_bind(fbo);
        
        GL_CHECK(glGenRenderbuffers(1, &fbo->render_buffer_id));
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, fbo->render_buffer_id));
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbo->screen_width, fbo->screen_height));
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo->render_buffer_id));

    __gl_framebuffer_unbind(fbo);
}


void gl_framebuffer_draw_quad(gl_framebuffer_t *fbo, gl_quad_t quad)
{
    if (fbo == NULL) eprint("fbo argument is null");

    GLenum status;
    if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
        eprint("Framebuffer Error: %s", glGetString(status));

    vao_t vao = vao_init(1);
    vbo_t vbo; 
    ebo_t ebo;

    vao_bind(&vao);

        vbo = vbo_init(&quad, sizeof(gl_quad_t));
        ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, DEFAULT_QUAD_INDICES_CAPACITY);

        vao_push(&vao, &vbo);
            vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position));
            //vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color));
            vao_set_attributes(&vao, 0, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord));

            __gl_framebuffer_bind_color_buffer_texture(fbo);
            gl_shader_bind(&fbo->color_buffer_shader);

            vao_draw(&vao);
        vao_pop(&vao);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
    vao_destroy(&vao);
}

void gl_framebuffer_destroy(gl_framebuffer_t *fbo)
{
    gl_shader_destroy(&fbo->color_buffer_shader);
    gl_texture_destroy(&fbo->color_buffer_texture);
    GL_CHECK(glDeleteFramebuffers(1, &fbo->frame_buffer_id));
}

#endif // __MY_GL_FRAMEBUFFER_H__
