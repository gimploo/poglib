#ifndef __MY_GL_FRAMEBUFFER_H__
#define __MY_GL_FRAMEBUFFER_H__

#include "./shader.h"
#include "./texture2d.h"

const char * const GL_FRAMEBUFFER_VS = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 v_pos;\n"
    "layout (location = 1) in vec3 v_color;\n"
    "layout (location = 2) in vec2 v_tex_coord;\n"
    "\n"
    "out vec3 color;\n"
    "out vec2 tex_coord;\n"
    "\n"
    "void main()\n"
    "{\n"
        "gl_Position = vec4(v_pos, 1.0f);\n"
        "tex_coord = v_tex_coord;\n"
        "color = v_color;\n"
    "}";

const char * const GL_FRAMEBUFFER_FS = 
    "#version 330 core\n"
    "in vec2 tex_coord;\n"
    "in vec3 color;\n"
    "\n"
    "uniform sampler2D u_color_texture;\n"
    "\n"
    "out vec4 FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
        "FragColor = texture(u_color_texture, tex_coord) + vec4(color, 1.0f);\n"
    "}";

typedef struct gl_framebuffer_t {

    GLuint id; // Frame buffer object id
    const u32 screen_width;
    const u32 screen_height;

    GLuint          rbo_id; // Render buffer object id
    gl_texture2d_t  color_texture; // Color attachment texture
    gl_shader_t     color_shader; // Color attachemnt shader 

} gl_framebuffer_t;

/*------------------------------------------------------------
 // Declarations
------------------------------------------------------------*/

gl_framebuffer_t    gl_framebuffer_init(u32 screen_width, u32 screen_height);
//NOTE:(macro)      gl_framebuffer_begin_scene(gl_framebuffer_t *) -> void
//NOTE:(macro)      gl_framebuffer_end_scene(gl_framebuffer_t *) -> void
void                gl_framebuffer_destroy(gl_framebuffer_t *);

/*------------------------------------------------------------
 // Implementation 
------------------------------------------------------------*/

#define __framebuffer_bind(pfbo)    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, (pfbo)->id))
#define __framebuffer_unbind(pfbo)  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0))

#define gl_framebuffer_begin_scene(pfbo) do {\
        __framebuffer_bind(pfbo);\
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));\
} while(0)

#define gl_framebuffer_end_scene(pfbo) do {\
        __framebuffer_unbind(pfbo);\
        GL_CHECK(glDisable(GL_DEPTH_TEST));\
} while(0)


//NOTE: this constructor creates the color attachemnt as the texture and both the depth and stencil buffer as render buffer object

gl_framebuffer_t gl_framebuffer_init(u32 screen_width, u32 screen_height)
{
    // Initializing the framebuffer
    GLuint fbo_id;
    GL_CHECK(glGenFramebuffers(1, &fbo_id));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_id));

    // Color attachemnt (texture)
    gl_texture2d_t color_texture = gl_texture2d_empty_init(screen_width, screen_height);
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture.id, 0));

    // Color attachemnt (shader)
    gl_shader_t color_shader = gl_shader_from_cstr_init(GL_FRAMEBUFFER_VS, GL_FRAMEBUFFER_FS);

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

    return (gl_framebuffer_t ) {
        .id            = fbo_id,
        .screen_width  = screen_width,
        .screen_height = screen_height,
        .rbo_id        = rbo_id,
        .color_texture = color_texture,
        .color_shader  = color_shader
    };
}

void gl_framebuffer_destroy(gl_framebuffer_t *fbo)
{
    GL_CHECK(glDeleteFramebuffers(1, &fbo->id));
    gl_texture2d_destroy(&fbo->color_texture);
    gl_shader_destroy(&fbo->color_shader);
}

#endif //__MY_GL_FRAMEBUFFER_H__
