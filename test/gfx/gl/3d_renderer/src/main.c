#include <poglib/application.h>
#include <poglib/math.h>
#include <poglib/util/glcamera.h>

typedef struct TEST {

    glshader_t  shader;
    gltexture2d_t texture;
    glcamera_t camera;


} TEST ;

void application_init(application_t *app)
{
    TEST test = (TEST){
        .shader     = glshader_from_file_init(
                        "lib/poglib/res/shaders/simple3d.vs", 
                        "lib/poglib/res/shaders/simple3d.fs"),
        .texture    = gltexture2d_init("res/pepe_ez.png"),
        .camera     = glcamera_perspective(
                        "view",
                        (vec3f_t ){ 0.0f, 0.0f, 6.0f })
    };

    application_pass_content(app, &test);
    matrix4f_t projection    = glms_mat4_identity();
    /*projection  = glms_perspective(*/
                        /*radians(45.0f), */
                        /*(f32 )app->window.aspect_ratio, */
                        /*0.1f, 100.0f);*/
    glshader_send_uniform_matrix4f(&test.shader, "projection", projection);

    matrix4f_t model         = glms_mat4_identity();
    model = glms_rotate(model, radians(20.0f), (vec3f_t ){1.0f, 0.3f, 0.5f});
    model = glms_translate(model, GLMS_VEC3_ZERO);
    glshader_send_uniform_matrix4f(&test.shader, "model", model);
}

void application_update(application_t *app)
{
    TEST *test                  = application_get_content(app);
    const glshader_t *shader    = &test->shader;

    window_update_user_input(application_get_window(app));
    glcamera_process_input(&test->camera, application_get_dt(app));

    glcamera_update(&test->camera, &test->shader);
}

void application_render(application_t *app)
{
    TEST *test = application_get_content(app);

    glrenderer3d_t rd3d = glrenderer3d(&test->shader, &test->texture);
    glrenderer3d_draw_cube(&rd3d);
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
