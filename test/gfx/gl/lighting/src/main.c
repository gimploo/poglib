#define WINDOW_SDL
#include <poglib/application.h>
#include <poglib/math.h>
#include <poglib/util/glcamera.h>

typedef struct TEST {

    glshader_t  shader;
    glcamera_t  camera;

    struct {
        vec3f_t pos;
        vec3f_t scale;
        vec4f_t color;
        glshader_t shader;
    } lighting;

} TEST ;


void application_init(application_t *app)
{
    TEST test = (TEST ){
        .shader     = glshader_from_file_init("res/object.vs", "res/object.fs"),
        .camera     = glcamera_perspective((vec3f_t ){ 0.6f, 1.0f, 5.0f}),
        .lighting = {
            .pos = (vec3f_t ){1.2f, 2.0f, 1.0f},
            .scale = vec3f(0.2f),
            .color = COLOR_WHITE,
            .shader = glshader_from_file_init("res/lighting.vs", "res/lighting.fs"),
        },
    };

    glshader_send_uniform_vec4f(&test.lighting.shader, "color", test.lighting.color);
    glshader_send_uniform_vec4f(&test.shader, "lightColor", test.lighting.color);

    application_pass_content(app, &test);


}

void application_update(application_t *app)
{
    TEST *test                  = application_get_content(app);
    const glshader_t *shader    = &test->shader;
    window_t *win               = application_get_window(app);

    window_update_user_input(application_get_window(app));
    glcamera_process_input(&test->camera, application_get_dt(app));

    if (window_keyboard_is_key_just_pressed(win, SDLK_UP))
        test->lighting.pos.y += 0.2f;
    if (window_keyboard_is_key_just_pressed(win, SDLK_DOWN))
        test->lighting.pos.y += -0.2f;
    if (window_keyboard_is_key_just_pressed(win, SDLK_RIGHT))
        test->lighting.pos.x += 0.2f;
    if (window_keyboard_is_key_just_pressed(win, SDLK_LEFT))
        test->lighting.pos.x += -0.2f;

    matrix4f_t view         = MATRIX4F_IDENTITY;
    matrix4f_t projection   = glms_perspective(
                                    radians(test->camera.zoom), 
                                    app->window.aspect_ratio, 
                                    0.1f, 
                                    100.0f
                              );

    glshader_send_uniform_matrix4f(
            &test->shader, 
            "view", 
            glcamera_getview(&test->camera)
    );
    glshader_send_uniform_matrix4f(&test->shader, "projection", projection);

    glshader_send_uniform_matrix4f(
            &test->lighting.shader, 
            "view", 
            glcamera_getview(&test->camera)
    );
    glshader_send_uniform_matrix4f(&test->lighting.shader, "projection", projection);

}

void application_render(application_t *app)
{
    TEST *test = application_get_content(app);

    matrix4f_t model = MATRIX4F_IDENTITY;
    model = glms_translate(model, vec3f(0.0f));
    /*model = glms_rotate(*/
                /*model, */
                /*radians(stopwatch_get_tick() / 150.0f), */
                /*(vec3f_t ){1.0f, 0.3f, 0.5f});*/
    glshader_send_uniform_matrix4f(&test->shader, "model", model);
    glshader_send_uniform_vec3f(&test->shader, "lightPos", test->lighting.pos);
    glrenderer3d_draw_cube(&(glrenderer3d_t ) {
            .shader = &test->shader,
            .texture = NULL
    });

    matrix4f_t lmodel = MATRIX4F_IDENTITY;
    lmodel = glms_translate(lmodel, test->lighting.pos);
    lmodel = glms_scale(lmodel, test->lighting.scale);
    glshader_send_uniform_matrix4f(&test->lighting.shader, "model", lmodel);
    glrenderer3d_draw_cube(&(glrenderer3d_t ) {
            .shader = &test->lighting.shader,
            .texture = NULL
    });
}

void application_destroy(application_t *app)
{
    TEST *test = application_get_content(app);
    glshader_destroy(&test->shader);
    glshader_destroy(&test->lighting.shader);
}


int main(void)
{
    application_t app = {

        .window = {
            .title = "lighting",
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

