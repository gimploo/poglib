#define WINDOW_GLFW
#include <poglib/application.h>

typedef struct content_t {

    gltexture2d_t texture;
    glshader_t shader;

} content_t ;

void coordinate_system_depth_init(application_t *app) 
{
    content_t cont = {
        .texture = gltexture2d_init("../coordinate_systems_multiple/res/pepe_ez.png"),
        .shader = glshader_from_file_init("lib/poglib/res/shaders/simple3d.vs", "lib/poglib/res/shaders/simple3d.fs"),
    };

    application_pass_content(app, &cont);
}

void coordinate_system_depth_update(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    window_update_user_input(win);

    matrix4f_t model = MATRIX4F_IDENTITY;
    matrix4f_t view = MATRIX4F_IDENTITY;
    matrix4f_t projection = MATRIX4F_IDENTITY;

    static f32 scale = -3.0f;
    if (window_keyboard_is_key_pressed(win, 'W'))
    {
        printf("-\n");
        scale -= 0.2f;
    }
    if (window_keyboard_is_key_pressed(win, 'S'))
    {
        printf("+\n");
        scale += 0.2f;
    }

    model = glms_rotate(model, (float)application_get_tick(), (vec3f_t ){0.5f, 1.0f, 0.0f});
    view  = glms_translate(view, (vec3f_t ){0.0f, 0.0f, scale});
    projection = glms_perspective(glm_rad(45.0f), app->window.aspect_ratio, 0.1f, 100.0f);

    glshader_send_uniform_matrix4f(
            &c->shader,
            "model",model);
    glshader_send_uniform_matrix4f(
            &c->shader,
            "view", view);
    glshader_send_uniform_matrix4f(
            &c->shader,
            "projection", projection);
}

void coordinate_system_depth_render(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    glrenderer3d_t rd3d = glrenderer3d(&c->shader, &c->texture);
    glrenderer3d_draw_cube(&rd3d);
}

void coordinate_system_depth_destroy(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    glshader_destroy(&c->shader);
    gltexture2d_destroy(&c->texture);
}

int main(void)
{
    application_t app = {
        .window = {
            .title = "coordinate_system_depth",
            .width = 800,
            .height = 600,
            .aspect_ratio = (f32)800 / (f32)600
        },   
        .content = {
            .size = sizeof(content_t )
        },
        .init       = coordinate_system_depth_init,
        .update     = coordinate_system_depth_update,
        .render     = coordinate_system_depth_render,
        .destroy    = coordinate_system_depth_destroy
    };

    application_run(&app);

    return 0;
}

