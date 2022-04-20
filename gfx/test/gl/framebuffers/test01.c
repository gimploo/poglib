#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "../../../gl_renderer2d.h"
#include "../../../window.h"

int main(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init("test01.c", 1080, 920, FLAGS);
    window_set_background(&window, (vec4f_t ) {1.0f, 0.0f, 0.0f, 1.0f});

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
    gl_texture2d_t texture = gl_texture2d_init("./wall.jpg");
    gl_renderer2d_t renderer = gl_renderer2d_init(&shader, &texture);

    gl_framebuffer_t fbo            = gl_framebuffer_init(window.width, window.height);
    const gl_quad_t fbo_quad        = gl_quad(
                                        quadf_init((vec2f_t ){0.0f, 1.0f}, 1.0f, 1.0f), 
                                        vec3f(0.0f), 
                                        quadf_init((vec2f_t ){0.0f, 1.0f}, 1.0f, 1.0f)); 

    window_while_is_open(&window)
    {
        window_gl_render_begin(&window);

            gl_framebuffer_begin_scene(&fbo);

                //NOTE: apparantly enabling depth test was 
                //fucking up the framebuffer and it would simply 
                //output a single color instead of a texture
                
                /*GL_CHECK(glEnable(GL_DEPTH_TEST));*/
                gl_renderer2d_draw_from_batch(&renderer, &batch);
            gl_framebuffer_end_scene(&fbo);

            gl_renderer2d_draw_frame_buffer(&fbo, fbo_quad);

        window_gl_render_end(&window);
    }

    gl_renderer2d_destroy(&renderer);
    window_destroy(&window);

    return 0;
}
