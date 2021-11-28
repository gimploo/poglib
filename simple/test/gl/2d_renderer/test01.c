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

    glquad_t quad = {    
                        // positon,            // color            // texture
      (glvertex_t){0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f}, 
      (glvertex_t){0.5f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0}, 
      (glvertex_t){0.5f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0}, 
      (glvertex_t){0.0f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0}, 
    };                      

    glshader_t shader = glshader_default_init();
    gltexture2d_t texture = gltexture2d_init("./wall.jpg");

    vao_t vao = vao_init(1);
    vbo_t vbo = vbo_init(&quad, sizeof(quad));
    ebo_t ebo = ebo_init(&vbo, DEFAULT_QUAD_INDICES, 6);
    vao_push(&vao, &vbo);

    vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 0);
    vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));
    vao_set_attributes(&vao, 0, 2, GL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));


    window_while_is_open(&window)
    {
        window_glrender_begin(&window);
            glshader_bind(&shader);
            gltexture2d_bind(&texture, 1);
            vao_draw(&vao);
        window_glrender_end(&window);
    }

    window_destroy(&window);

    return 0;
}
#endif

int main(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init("new_main", 1080, 920, FLAGS);

    const glquad_t quads[2] = {    
                        // positon,            // color            // texture
        (glvertex_t) {0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 0.0f},
        (glvertex_t) {0.5f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f},
        (glvertex_t) {0.5f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f},
        (glvertex_t) {0.0f, 0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f},

        (glvertex_t) {-0.5f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f},
        (glvertex_t) {0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 0.0f} ,
        (glvertex_t) {0.0f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f},
        (glvertex_t) {-0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f},


    };                      


    glbatch_t batch = glbatch_init(quads, sizeof(quads) * 2, glquad_t );

    glshader_t shader = glshader_from_file_init("./wood.vs", "./wood.fs");
    gltexture2d_t texture = gltexture2d_init("./wall.jpg");
    glrenderer2d_t renderer = glrenderer2d_init(&shader, &texture);


    while(window.is_open)
    {
        window_update_user_input(&window);
        window_gl_render_begin(&window);
            glrenderer2d_draw_from_batch(&renderer, &batch);
            /*glrenderer2d_draw_quad(&renderer, quads[0]);*/
        window_gl_render_end(&window);
    }
    SDL_Log("Nah\n");

    glbatch_destroy(&batch);
    glrenderer2d_destroy(&renderer);
    window_destroy(&window);

    return 0;
}
