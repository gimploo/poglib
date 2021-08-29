#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../../../gl_renderer2d.h"
#include "../../../gl_framebuffer.h"

#include "../../../window.h"

int main(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init("new_main", 1080, 920, FLAGS);

    const gl_quad_t quad[2] = {    
                        // positon,            // color            // texture
        (gl_vertex_t) {0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 1.0f},
        (gl_vertex_t) {0.5f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 0.0f},
        (gl_vertex_t) {0.5f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f},
        (gl_vertex_t) {0.0f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f},

        (gl_vertex_t) {-0.5f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 0.0f},
        (gl_vertex_t) {0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 1.0f} ,
        (gl_vertex_t) {0.0f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f},
        (gl_vertex_t) {-0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f},

    };                      

    gl_batch_t batch = { 

        .shape_type = BT_QUAD,
        .shape_count = 2,
        .vertex_buffer= (gl_vertex_t *)quad,
        .vertex_buffer_size= sizeof(quad),
    };

    gl_shader_t shader = gl_shader_from_file_init("./wood.vs", "./wood.fs");
    gl_texture2d_t texture = gl_texture_init("./wall.jpg");
    gl_renderer2d_t renderer = gl_renderer2d_init(&shader, &texture);

    gl_framebuffer_t fbo = gl_framebuffer_init(1080, 920);
    gl_shader_t shader02 = gl_shader_from_file_init("./wood.vs", "./wood.fs");
    gl_framebuffer_attach_color_texture(&fbo, shader02, "u_texture01");
    gl_framebuffer_attach_render_buffer(&fbo);

    const gl_quad_t fbo_quad = gl_quad(
            quadf_init((vec2f_t ){-1.0f, -0.2f}, 0.4f, 0.4f), 
            vec3f(1.0f), 
            quadf_init((vec2f_t ){-1.0f, 1.0f}, 2.0f, 2.0f));

    window_while_is_open(&window)
    {
        gl_framebuffer_begin_scene(&fbo);
            gl_renderer2d_draw_from_batch(&renderer, &batch);
        gl_framebuffer_end_scene(&fbo);

        window_gl_render_begin(&window);
            gl_renderer2d_draw_from_batch(&renderer, &batch);
            gl_framebuffer_draw_quad(&fbo, fbo_quad);
        window_gl_render_end(&window);
    }

    gl_framebuffer_destroy(&fbo);
    gl_renderer2d_destroy(&renderer);
    window_destroy(&window);

    return 0;
}
