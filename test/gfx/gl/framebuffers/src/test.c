#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <poglib/application.h>

int main(void)
{
    dbg_init();
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t * window = window_init("test01.c", 1080, 920, FLAGS);
    window_set_background(window, (vec4f_t ) {0.0f, 0.0f, 0.0f, 1.0f});

    const glquad_t quad[2] = {    
                        // positon,            // color            // texture
        (glvertex_t) {0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 0},
        (glvertex_t) {0.5f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0},
        (glvertex_t) {0.5f, 0.5f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0},
        (glvertex_t) {0.0f, 0.5f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0},

        (glvertex_t) {-0.5f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,0.9f,   1.0f, 0.0f, 0},
        (glvertex_t) {0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.8f,  1.0f, 1.0f, 0} ,
        (glvertex_t) {0.0f, -0.5f, 1.0f,  0.0f, 1.0f, 0.0f,0.1f,   0.0f, 0.0f, 0},
        (glvertex_t) {-0.5f, -0.5f, 1.0f,  0.0f, 1.0f, 0.0f,0.4f,  0.0f, 1.0f, 0},

    };                      

    glbatch_t batch = glbatch_init(2, glquad_t );


    /*glshader_t shader = glshader_from_file_init("./wood.vs", "./wood.fs");*/
    glshader_t shader = glshader_default_init();
    gltexture2d_t texture = gltexture2d_init("./wall.jpg");
    glrenderer2d_t renderer = glrenderer2d_init(&shader, &texture);

    glframebuffer_t fbo            = glframebuffer_init(window->width, window->height);
    const glquad_t fbo_quad        = glquad(
                                        quadf(vec3f(0.0f), 1.0f, 1.0f), 
                                        vec4f(0.0f), 
                                        quadf((vec3f_t ){0.0f, 1.0f, 1.0f}, 1.0f, 1.0f), 0);

    glbatch_put(&batch, quad[0]);
    glbatch_put(&batch, quad[1]);

    glrenderer2d_t rd2d = glrenderer2d_init(
            &shader,
            &fbo.texture2d);

    while(window->is_open)
    {
        window_update_user_input(window);
        window_set_background(window, (vec4f_t ) {0.0f, 0.0f, 0.0f, 1.0f});
        window_gl_render_begin(window);

            glframebuffer_begin_texture(&fbo);

                //NOTE: apparantly enabling depth test was 
                //fucking up the framebuffer and it would simply 
                //output a single color instead of a texture
                
                glrenderer2d_draw_from_batch(&renderer, &batch);
                /*GL_CHECK(glEnable(GL_DEPTH_TEST));*/
            glframebuffer_end_texture(&fbo);

            glrenderer2d_draw_quad(&rd2d, fbo_quad);

        window_gl_render_end(window);
    }

    glshader_destroy(&shader);
    glbatch_destroy(&batch);
    glframebuffer_destroy(&fbo);
    glrenderer2d_destroy(&renderer);
    glrenderer2d_destroy(&rd2d);
    window_destroy();

    dbg_destroy();

    return 0;
}
