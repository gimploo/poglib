#ifndef __MY_GL_FRAMEBUFFER_H__
#define __MY_GL_FRAMEBUFFER_H__

#include "./gl_renderer2d.h"

/*====================================================================================
 // OpenGL Framebuffer 
====================================================================================*/

typedef struct gl_framebuffer_t {

    GLuint          id;
    GLuint          render_buffer_id;
    u32             screen_width; 
    u32             screen_height;
    
    gl_shader_t     color_texture_shader;
    gl_texture2d_t  color_texture;


} gl_framebuffer_t;

/*------------------------------------------------------------------------------
 // Declaration
------------------------------------------------------------------------------*/

gl_framebuffer_t    gl_framebuffer_init(u32 window_width, u32 window_height);
void                gl_framebuffer_attach_color_texture(gl_framebuffer_t *fbo, gl_shader_t shader, const char *unifrom); //NOTE: use this function if your planning to use a shader with this texture
void                gl_framebuffer_attach_render_buffer(gl_framebuffer_t *fbo); //NOTE: faster and isnt accessable from a shader

//NOTE:(macro)      gl_framebuffer_begin_scene(framebuffer_t *fbo) -> void
//NOTE:(macro)      gl_framebuffer_end_scene(framebuffer_t *fbo) -> void

void                gl_framebuffer_draw_quad(gl_framebuffer_t *fbo, gl_quad_t quad);
void                gl_framebuffer_destroy(gl_framebuffer_t *fbo);

/*------------------------------------------------------------------------------
 // Implementation
------------------------------------------------------------------------------*/

gl_framebuffer_t gl_framebuffer_init(u32 window_width, u32 window_height)
{
    gl_framebuffer_t fbo;
    fbo.screen_width = window_width;
    fbo.screen_height = window_height;
    fbo.color_texture = (gl_texture2d_t ){0};
    GL_CHECK(glGenFramebuffers(1, &fbo.id));
    return fbo;
}

#define __gl_framebuffer_bind(pfbo)         GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, (pfbo)->id))
#define __gl_framebuffer_unbind(pfbo)       GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0 ))


void gl_framebuffer_attach_color_texture(gl_framebuffer_t *fbo, gl_shader_t shader, const char *uniform_name)
{
    assert(uniform_name);

    // Shader
    fbo->color_texture_shader = shader;
    int value = 0;
    gl_shader_set_uniform(&fbo->color_texture_shader, uniform_name, &value, sizeof(int), UT_INT);

    //Texture
    __gl_framebuffer_bind(fbo);

        GL_CHECK(glGenTextures(1, &fbo->color_texture.id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, fbo->color_texture.id));
        GL_CHECK(glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, fbo->screen_width, fbo->screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));	
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->color_texture.id, 0));

    __gl_framebuffer_unbind(fbo);

}


void gl_framebuffer_attach_render_buffer(gl_framebuffer_t *fbo)
{
    __gl_framebuffer_bind(fbo);
        
        GL_CHECK(glGenRenderbuffers(1, &fbo->render_buffer_id));
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, fbo->render_buffer_id));
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbo->screen_width, fbo->screen_height));
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo->render_buffer_id));

    __gl_framebuffer_unbind(fbo);
}

//NOTE: the window gl begin api call clears the colour bits
#define gl_framebuffer_begin_scene(pfbo) do {\
    __gl_framebuffer_bind(pfbo);\
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);\
    glEnable(GL_DEPTH_TEST);\
} while(0)

#define gl_framebuffer_end_scene(pfbo) do {\
    __gl_framebuffer_unbind(pfbo);\
    glDisable(GL_DEPTH_TEST);\
} while(0)
    
#define __gl_framebuffer_use_color_texture(pfbo) GL_CHECK(glBindTexture(GL_TEXTURE_2D, (pfbo)->color_texture.id))

void gl_framebuffer_draw_quad(gl_framebuffer_t *fbo, gl_quad_t quad)
{
    if (fbo == NULL) eprint("fbo argument is null");

    GLenum status;
    if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
        eprint("Framebuffer Error: %s", glGetString(status));

    gl_renderer2d_t renderer = gl_renderer2d_init(&fbo->color_texture_shader, &fbo->color_texture);

    vbo_t vbo; 
    ebo_t ebo;

    vao_bind(&renderer.vao);

        vbo = vbo_init(&quad, sizeof(gl_quad_t));
        ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, DEFAULT_QUAD_INDICES_CAPACITY);

        vao_push(&renderer.vao, &vbo);
            vao_set_attributes(&renderer.vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position));
            vao_set_attributes(&renderer.vao, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color));
            vao_set_attributes(&renderer.vao, 0, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord));

            __gl_framebuffer_use_color_texture(fbo);

            gl_shader_bind(renderer.shader);
            vao_draw(&renderer.vao);
        vao_pop(&renderer.vao);

    vao_unbind();

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
    gl_renderer2d_destroy(&renderer);
}

void gl_framebuffer_destroy(gl_framebuffer_t *fbo)
{
    gl_shader_destroy(&fbo->color_texture_shader);
    gl_texture_destroy(&fbo->color_texture);
    GL_CHECK(glDeleteFramebuffers(1, &fbo->id));
}

#endif // __MY_GL_FRAMEBUFFER_H__
