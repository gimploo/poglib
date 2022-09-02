#include <poglib/application.h>
#include <poglib/math.h>
/*#include <poglib/util/glcamera.h>*/

typedef struct TEST {

    glshader_t  shader;
    gltexture2d_t texture;
    /*glcamera_t camera;*/


} TEST ;

void application_init(application_t *app)
{
    TEST test = (TEST){
        .shader     = glshader_from_file_init(
                        "lib/poglib/res/shaders/simple3d.vs", 
                        "lib/poglib/res/shaders/simple3d.fs"),
        .texture    = gltexture2d_init("res/texture.jpg"),
        /*.camera     = glcamera_perspective("mvp",*/
                                /*(vec3f_t ){0.0f, 0.0f, 1.0f}, */
                                /*70, */
                                /*app->window.aspect_ratio)*/
    };

    application_pass_content(app, &test);
}

void application_update(application_t *app)
{
    TEST *test = application_get_content(app);
    glshader_t *shader = &test->shader;

    window_update_user_input(application_get_window(app));

    matrixf_t transform = matrix4f_identity();
    transform = matrix4f_rotate(transform, radians(SDL_GetTicks()/4), (vec3f_t ){0.0f, 0.0f, 1.0f});
    transform = matrix4f_translate(transform, (vec3f_t ){0.5f, -0.5f, 0.0f});

    /*matrixf_t model = matrixf_product(*/
                        /*matrix4f_translation((vec3f_t ){0.0f, 0.0f, -2.0f}),*/
                        /*matrixf_product(matrix4f_rotation(radians(10.0f), 'y'), matrix4f_rotation(radians(10.0f), 'x'))*/
                      /*);*/
    /*glshader_send_uniform_matrix4f(shader, "model", model);*/
    /*printf("\n MODEL := \n");*/
    /*matrixf_print(&model);*/
    glshader_send_uniform_matrix4f(shader, "model", transform);


    /*matrixf_t view = matrix4f_lookat(*/
                        /*(vec3f_t ){0.0f, 2.0f, 0.0f},*/
                        /*(vec3f_t ){0.0f, 0.0f, -4.0f},*/
                        /*(vec3f_t ){0.0f, 1.0f, 0.0f}*/
                     /*);*/
    /*matrixf_t view = matrix4f_identity();*/
    /*glshader_send_uniform_matrix4f(shader, "view", view);*/
    /*printf("\n VIEW := \n");*/
    /*matrixf_print(&view);*/

    /*matrixf_t projection = matrix4f_perpective(*/
                                /*radians(45.0f), */
                                /*app->window.aspect_ratio, */
                                /*0.1f, 100.0f);*/
    /*glshader_send_uniform_matrix4f(shader, "projection", projection);*/
    /*printf("\n PROJECTION := \n");*/
    /*matrixf_print(&projection);*/
}

void application_render(application_t *app)
{
    TEST *test = application_get_content(app);

    /*glrenderer3d_t rd3d = glrenderer3d(&test->shader, &test->texture);*/
    /*glrenderer3d_draw_cube(&rd3d);*/

    glrenderer2d_t rd2d = glrenderer2d(&test->shader, NULL);
    glquad_t quad = glquad(quadf((vec3f_t ){-0.2f, 0.2f, 0.0f},0.4f, 0.4f), COLOR_RED, quadf(vec3f(0.0f), 0.0f, 0.0f), 0);
    glrenderer2d_draw_quad(&rd2d, quad);
}

void application_destroy(application_t *app)
{
    TEST *test = application_get_content(app);
    glshader_destroy(&test->shader);
    gltexture2d_destroy(&test->texture);
}


int main(void)
{
    application_t app = {

        .window = {
            .title = "3d renderer test",
            .width = 800,
            .height = 800,
            .aspect_ratio = 800 / 800
        },

        .content = {
            .size = sizeof(TEST),
        },

        .init = application_init,
        .update = application_update,
        .render = application_render,
        .destroy = application_destroy
    };

    application_run(&app);

    return 0;
}
