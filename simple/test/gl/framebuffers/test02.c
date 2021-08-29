#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#define GL_LOG_ENABLE
#include "../../../gl_renderer2d.h"

#include "../../../window.h"

int main(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init("test.c",1080, 920, FLAGS);

    const gl_quad_t quad = {    
                        // positon,            // color            // texture
        (gl_vertex_t) {0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f},
        (gl_vertex_t) {0.5f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f},
        (gl_vertex_t) {0.5f, 0.5f, 0.0f,  1.0f, 1.0f, 0.0f,   0.0f, 0.0f},
        (gl_vertex_t) {0.0f, 0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f},

    };                      


    quadf_t rectangle = {
        0.0f, 0.0f,  
        0.5f, 0.0f,  
        0.5f, 0.5f,  
        0.0f, 0.5f,  
    };

    trif_t triangle = {
        0.0f, 0.0f,  
        -0.5f, 0.0f,  
        -0.5f, 0.5f,  
    };


    const gl_quad_t qua = gl_quad(rectangle, (vec3f_t ) {1.0f, 0.0f, 0.0f}, quadf(0));
    const gl_tri_t tri = gl_tri(triangle, (vec3f_t ) {1.0f, 1.0f, 0.0f}, quadf(0));

    gl_shader_t shader = gl_shader_from_file_init("./wood.vs", "./wood.fs");
    int i = 0;
    gl_texture2d_t texture = texture_init("./wall.jpg");
    gl_renderer2d_t renderer = gl_renderer2d_init(&shader, &texture);


    while (window.is_open)
    {
        window_gl_render_begin(&window);
            gl_renderer2d_draw_quad(&renderer, qua);
            gl_renderer2d_draw_triangle(&renderer, tri);
        window_gl_render_end(&window);
    }

    gl_render2d_destroy(&renderer);
    window_destroy(&window);

    return 0;
}
