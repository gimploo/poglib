#define WINDOW_SDL
#include <poglib/application.h>
#include <poglib/math.h>
#include <poglib/util/glcamera.h>

typedef struct TEST {

    glshader_t    shader;
    gltexture2d_t texture;
    glcamera_t  camera;

} TEST ;


void application_init(application_t *app)
{
    TEST test = (TEST ){
        .shader     = glshader_from_file_init(
                        "lib/poglib/res/shaders/simple3d.vs", 
                        "lib/poglib/res/shaders/simple3d.fs"),
        .texture    = gltexture2d_init("res/pepe_ez.png"),
        .camera     = glcamera_perspective((vec3f_t ){ 0.0f, 0.0f, 3.0f})
    };

    application_pass_content(app, &test);

    matrix4f_t projection = MATRIX4F_IDENTITY;
    projection = glms_perspective(
                    radians(45.0f), app->window.aspect_ratio, 0.1f, 100.0f);
    glshader_send_uniform_matrix4f(&test.shader, "projection", projection);

}

void application_update(application_t *app)
{
    TEST *test                  = application_get_content(app);
    const glshader_t *shader    = &test->shader;
    window_t *win               = application_get_window(app);

    window_update_user_input(application_get_window(app));
    /*glcamera_update(&test->camera);*/
    /*glcamera_process_input(&test->camera, application_get_dt(app));*/

    matrix4f_t view = MATRIX4F_IDENTITY;
    glshader_send_uniform_matrix4f(
            &test->shader, "view", 
            /*glcamera_getview(&test->camera));*/
            glms_translate(view, (vec3f_t ){0.0f, 0.0f, -3.0f}));

}

void application_render(application_t *app)
{
    TEST *test = application_get_content(app);

    glrenderer3d_t rd3d = glrenderer3d(&test->shader, &test->texture);

    vec3f_t cubePositions[] = {
        (vec3f_t ){ 0.0f,  0.0f,  0.0f},
        (vec3f_t ){-1.5f, -2.2f, -2.5f},
        (vec3f_t ){-3.8f, -2.0f, -12.3f},
        (vec3f_t ){ 2.4f, -0.4f, -3.5f},
        (vec3f_t ){-1.7f,  3.0f, -7.5f},
        (vec3f_t ){ 1.3f, -2.0f, -2.5f},
        (vec3f_t ){ 1.5f,  2.0f, -2.5f},
        (vec3f_t ){ 1.5f,  0.2f, -1.5f},
        (vec3f_t ){-1.3f,  1.0f, -1.5f},
        (vec3f_t ){ 2.0f,  5.0f, -15.0f},
    };

    for (unsigned int i = 0; i < ARRAY_LEN(cubePositions); i++)
    {
        matrix4f_t model = MATRIX4F_IDENTITY;
        model = glms_translate(model, cubePositions[i]);
        model = glms_rotate(
                    model, 
                    radians(20.0f * i), 
                    (vec3f_t ){1.0f, 0.3f, 0.5f});
        glshader_send_uniform_matrix4f(&test->shader, "model", model);
        glrenderer3d_draw_cube(&rd3d);
    }
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
            .height = 600,
            .aspect_ratio = (f32)800 / (f32)600,
            .fps_limit = 60
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

