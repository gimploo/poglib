#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#define GL_LOG_ENABLE
#include "../../../gl_renderer.h"

#include "../../../window.h"

int main(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init(1080, 920, FLAGS);

    const gl_quad_t quad = {    
                        // positon,            // color            // texture
        (gl_vertex_t) {0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f},
        (gl_vertex_t) {0.5f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f},
        (gl_vertex_t) {0.5f, 0.5f, 0.0f,  1.0f, 1.0f, 0.0f,   0.0f, 0.0f},
        (gl_vertex_t) {0.0f, 0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f},

    };                      

    gl_shader_t shader = gl_shader_from_file_init("./wood.vs", "./wood.fs");
    int i = 0;
    gl_shader_push_uniform(&shader, "u_texture01", &i, sizeof(int), UT_INT);
    gl_texture2d_t texture = texture_init("./wall.jpg");
    gl_renderer2d_t renderer = gl_renderer2d_init(&shader, &texture);


    while (window.is_open)
    {
        window_gl_render_begin(&window);
            gl_renderer2d_draw_quad(&renderer, quad);
        window_gl_render_end(&window);
    }

    gl_render2d_destroy(&renderer);
    window_destroy(&window);

    return 0;
}
