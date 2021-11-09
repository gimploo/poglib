#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../../../gl_renderer2d.h"

#include "../../../window.h"

#if 0
int oldmain(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init("old_main", 1080, 920, FLAGS);

    gl_quad_t quad = {    
                        // positon,            // color            // texture
      (gl_vertex_t){0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f}, 
      (gl_vertex_t){0.5f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0}, 
      (gl_vertex_t){0.5f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0}, 
      (gl_vertex_t){0.0f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0}, 
    };                      

    gl_shader_t shader = gl_shader_default_init();
    gl_texture2d_t texture = gl_texture2d_init("./wall.jpg");

    vao_t vao = vao_init(1);
    vbo_t vbo = vbo_init(&quad, sizeof(quad));
    ebo_t ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, 6);
    vao_push(&vao, &vbo);

    vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 0);
    vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));
    vao_set_attributes(&vao, 0, 2, GL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));


    window_while_is_open(&window)
    {
        window_gl_render_begin(&window);
            gl_shader_bind(&shader);
            gl_texture2d_bind(&texture, 1);
            vao_draw(&vao);
        window_gl_render_end(&window);
    }

    window_destroy(&window);

    return 0;
}
#endif

int main(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init("new_main", 1080, 920, FLAGS);

    const gl_quad_t quads[2] = {    
                        // positon,            // color            // texture
        (gl_vertex_t) {0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 0.0f},
        (gl_vertex_t) {0.5f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f},
        (gl_vertex_t) {0.5f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f},
        (gl_vertex_t) {0.0f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f},

        (gl_vertex_t) {-0.5f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f},
        (gl_vertex_t) {0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 0.0f} ,
        (gl_vertex_t) {0.0f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f},
        (gl_vertex_t) {-0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f},

    };                      


    gl_batch_t batch = gl_batch_init(quads, gl_quad_t );

    gl_shader_t shader = gl_shader_from_file_init("./wood.vs", "./wood.fs");
    gl_texture2d_t texture = gl_texture2d_init("./wall.jpg");
    gl_renderer2d_t renderer = gl_renderer2d_init(&shader, &texture);


    while(window.is_open)
    {
        window_update_user_input(&window);
        window_gl_render_begin(&window);
            gl_renderer2d_draw_from_batch(&renderer, &batch);
            /*gl_renderer2d_draw_quad(&renderer, quad[0]);*/
        window_gl_render_end(&window);
    }
    SDL_Log("Nah\n");

    gl_batch_destroy(&batch);
    gl_renderer2d_destroy(&renderer);
    window_destroy(&window);

    return 0;
}
