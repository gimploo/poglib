#include <poglib/application.h>
#include <poglib/math.h>

typedef struct TEST {

    glshader_t    shader;
    gltexture2d_t texture;

} TEST ;


void application_init(application_t *app)
{
    TEST test = (TEST ){
        .shader     = glshader_from_file_init(
                        "lib/poglib/res/shaders/simple3d.vs", 
                        "lib/poglib/res/shaders/simple3d.fs"),
        .texture    = gltexture2d_init("res/pepe_ez.png"),
    };

    application_pass_content(app, &test);

    matrix4f_t projection = glms_mat4_identity();
    projection = glms_perspective(
            glm_rad(90.0f), app->window.aspect_ratio, 0.5f, 1000.0f);
    glshader_send_uniform_matrix4f(&test.shader, "projection", projection);

}

void application_update(application_t *app)
{
    TEST *test                  = application_get_content(app);
    const glshader_t *shader    = &test->shader;
    window_t *win               = application_get_window(app);

    static f32 scale = -3.0f;
    if (window_keyboard_is_key_pressed(win, 'w'))
        scale += 0.08f;
    if (window_keyboard_is_key_pressed(win, 's'))
        scale -= 0.08f;

    window_update_user_input(application_get_window(app));
    glshader_send_uniform_matrix4f(
            &test->shader, 
            "translation", 
            matrix4f_translation((vec3f_t ){0.0f, 0.0f, scale}));

}

void application_render(application_t *app)
{
    TEST *test = application_get_content(app);

    glrenderer3d_t rd3d = glrenderer3d(&test->shader, &test->texture);

    static f32 scale = 0.0f;
    scale += 0.01f;
    glshader_send_uniform_matrix4f(
            &test->shader,
            "rotation",
            glms_rotate(glms_mat4_identity(), scale, (vec3f_t ){0.5f, 0.2f, 0.2f}));

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
            .aspect_ratio = (f32)800 / (f32)800,
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

