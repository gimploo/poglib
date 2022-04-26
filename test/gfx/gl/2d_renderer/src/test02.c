#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

/*#define GL_LOG_ENABLE*/
#include <poglib/application.h>



int main(void)
{
    dbg_init();
    u32 FLAGS = SDL_INIT_VIDEO;
    window_t *window = window_init("test.c",1080, 920, FLAGS);


    trif_t triangle01 = trif(vec3f(0.0f), 0.5f);
    trif_t triangle02 = trif(vec3f(0.5f), 0.5f);

    quadf_t quad02 = quadf(vec3f(0.0f), 0.2f, 0.2f);
    quadf_t quad03 = quadf(vec3f(0.5f), 0.2f, 0.2f);

    circle_t circle01 = circle(vec3f(0.0f), 0.2f);
    circle_t circle02 = circle(vec3f(0.5f), 0.2f);


    gltri_t tri01 = gltri(triangle01, (vec4f_t ) {1.0f, 1.0f, 0.0f, 1.0f}, quadf(vec3f(0.0f), 1.0f, 1.0f), 0);
    gltri_t tri02 = gltri(triangle02, (vec4f_t ) {1.0f, 1.0f, 0.0f, 1.0f}, quadf(vec3f(0.0f), 1.0f, 1.0f), 0);

    glquad_t glquad01 = glquad(quad02, (vec4f_t ) {1.0f, 1.0f, 0.0f, 1.0f}, quadf(vec3f(0.0f), 1.0f, 1.0f), 0);
    glquad_t glquad02 = glquad(quad03, (vec4f_t ) {1.0f, 1.0f, 0.0f, 1.0f}, quadf(vec3f(0.0f), 1.0f, 1.0f), 0);

    glcircle_t glcircle01 = glcircle(circle01, (vec4f_t ) {1.0f, 1.0f, 1.0f, 1.0f}, quadf(vec3f(0.0f), 1.0f, 1.0f), 0);
    glcircle_t glcircle02 = glcircle(circle02, (vec4f_t ) {1.0f, 0.0f, 1.0f, 1.0f}, quadf(vec3f(0.0f), 1.0f, 1.0f), 0);

    /*glshader_t shader = glshader_from_file("./wood.vs", "./wood.fs");*/
    glshader_t shader = glshader_default_init();
    gltexture2d_t texture = gltexture2d_init("./wall.jpg");
    glrenderer2d_t renderer = {
        .shader = &shader,
        .texture = &texture
    };

    glbatch_t batchtri = glbatch_init(2,  gltri_t );
    glbatch_put(&batchtri, tri01);
    glbatch_put(&batchtri, tri02);

    glbatch_t batchquads = glbatch_init(2,  glquad_t );
    glbatch_put(&batchquads, glquad01);
    glbatch_put(&batchquads, glquad02);

    glbatch_t batchcircles = glbatch_init(2, glcircle_t );
    glbatch_put(&batchcircles, glcircle01);
    glbatch_put(&batchcircles, glcircle02);

    while(window->is_open)
    {
        window_update_user_input(window);
        window_gl_render_begin(window);
            /*glrenderer2d_draw_quad(&renderer, glquad01);*/
            /*glrenderer2d_draw_triangle(&renderer, tri01);*/
            /*glrenderer2d_draw_circle(&renderer, glcircle01);*/
            glrenderer2d_draw_from_batch(&renderer, &batchtri);
            glrenderer2d_draw_from_batch(&renderer, &batchquads);
            glrenderer2d_draw_from_batch(&renderer, &batchcircles);
        window_gl_render_end(window);
    }

    glshader_destroy(&shader);
    gltexture2d_destroy(&texture);

    glbatch_destroy(&batchtri);
    glbatch_destroy(&batchquads);
    glbatch_destroy(&batchcircles);

    window_destroy();

    dbg_destroy();
    return 0;
}

