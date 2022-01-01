#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#define GL_LOG_ENABLE
#include "../../../glrenderer2d.h"

#include "../../../window.h"

int main(void)
{
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init("test.c",1080, 920, FLAGS);

    const glquad_t quad = {    
                        // positon,            // color            // texture
        (glvertex_t) {0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f},
        (glvertex_t) {0.5f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f},
        (glvertex_t) {0.5f, 0.5f, 0.0f,  1.0f, 1.0f, 0.0f,   0.0f, 0.0f},
        (glvertex_t) {0.0f, 0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f},

    };                      


    quadf_t rectangle = {
        0.0f, 0.0f,  
        0.5f, 0.0f,  
        0.5f, 0.5f,  
        0.0f, 0.5f,  
    };

    /*trif_t triangle = {*/
        /*0.0f, 0.0f,  */
        /*-0.5f, 0.0f,  */
        /*-0.5f, 0.5f,  */
    /*};*/

    trif_t triangle01 = trif_init(vec2f(0.0f), 0.5f);
    trif_t triangle02 = trif_init(vec2f(0.5f), 0.5f);


    const glquad_t qua = glquad_init(rectangle, (vec3f_t ) {1.0f, 0.0f, 0.0f}, quadf_init(vec2f(0.0f), 1.0f, 1.0f), 0);
    const gltri_t tri01 = gltri_init(triangle01, (vec3f_t ) {1.0f, 1.0f, 0.0f}, quadf_init(vec2f(0.0f), 1.0f, 1.0f));
    const gltri_t tri02 = gltri_init(triangle02, (vec3f_t ) {1.0f, 1.0f, 0.0f}, quadf_init(vec2f(0.0f), 1.0f, 1.0f));

    glshader_t shader = glshader_from_file_init("./wood.vs", "./wood.fs");
    int i = 0;
    gltexture2d_t texture = gltexture2d_init("./wall.jpg");
    glrenderer2d_t renderer = glrenderer2d_init(&shader, &texture);

    gltri_t triangles[2] = {tri01, tri02};
    glbatch_t batchtri = glbatch_init(&triangles, sizeof(triangles),  gltri_t );

    while(window.is_open)
    {
        window_update_user_input(&window);
        window_gl_render_begin(&window);
            /*glrenderer2d_draw_quad(&renderer, qua);*/
            /*glrenderer2d_draw_triangle(&renderer, tri02);*/
            glrenderer2d_draw_from_batch(&renderer, &batchtri);
        window_gl_render_end(&window);
    }

    glbatch_destroy(&batchtri);
    glrenderer2d_destroy(&renderer);
    window_destroy(&window);

    return 0;
}
