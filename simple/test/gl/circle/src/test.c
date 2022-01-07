#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
/*#define GL_LOG_ENABLE*/
#include "../../../../glrenderer2d.h"
#include "../../../../window.h"


int main(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init("test.c",1080, 920, FLAGS);

    glshader_t shader = glshader_from_file_init("./assets/wood.vs", "./assets/wood.fs");
    gltexture2d_t texture = gltexture2d_init("./assets/wall.jpg");
    glrenderer2d_t renderer = glrenderer2d_init(&shader, &texture);

    circle_t circle = circle_init(vec3f(0.0f), 0.4f);
    glcircle_t glcircle = glcircle_init(circle, (vec4f_t ) {1.0f, 0.0f, 1.0f, 1.0f}, quadf_init(vec2f(0.0f), 1.0f, 1.0f), 0);

    while(window.is_open)
    {
        window_update_user_input(&window);
        window_gl_render_begin(&window);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glrenderer2d_draw_circle(&renderer, glcircle);
        window_gl_render_end(&window);
    }

    /*glbatch_destroy(&batchtri);*/
    glrenderer2d_destroy(&renderer);
    window_destroy(&window);

    return 0;
}
