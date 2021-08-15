#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../../../gl_renderer.h"

#include "../../../window.h"

int oldmain(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init(1080, 920, FLAGS);

    float vertices[] = {    
                        // positon,            // color            // texture
        0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f ,
        0.5f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
        0.5f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
        0.0f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f,
    };                      

    gl_shader_t shader = shader_default_init();
    gl_texture2d_t texture = texture_init("./glyph_atlas.png");

    vao_t vao = vao_init(1);
    vbo_t vbo = vbo_init(vertices, sizeof(vertices));
    ebo_t ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, 6);
    vao_push(&vao, &vbo);

    vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 0);
    vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));
    vao_set_attributes(&vao, 0, 2, GL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));


    while (window.is_open)
    {
        window_process_user_input(&window);
        window_gl_render_begin(&window);
            shader_bind(&shader);
            texture_bind(&texture, 0);
            vao_draw(&vao);
        window_gl_render_end(&window);
    }

    window_destroy(&window);

    return 0;
}

int main(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init(1080, 920, FLAGS);

    const gl_quad_t quads[2] = {    
                        // positon,            // color            // texture
        (gl_vertex_t) {0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f} ,
        (gl_vertex_t) {0.5f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f},
        (gl_vertex_t) {0.5f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f},
        (gl_vertex_t) {0.0f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f},

        (gl_vertex_t) {-0.5f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f},
        (gl_vertex_t) {0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f} ,
        (gl_vertex_t) {0.0f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f},
        (gl_vertex_t) {-0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f},

    };                      

    gl_batch_t batch = (gl_batch_t) { 

        .shape_type = BT_QUAD,
        .shape_count = 2,
        .vertex_buffer= quads,
        .vertex_buffer_size= sizeof(quads),
    };

    gl_shader_t shader = shader_default_init();
    gl_texture2d_t texture = texture_init("./wall.jpg");
    gl_renderer2d_t renderer = gl_renderer2d_init(&shader, &texture);


    while (window.is_open)
    {
        window_process_user_input(&window);
        window_gl_render_begin(&window);
            gl_renderer2d_draw_from_batch(&renderer, &batch);
        window_gl_render_end(&window);
    }

    gl_render2d_destroy(&renderer);
    window_destroy(&window);

    return 0;
}
