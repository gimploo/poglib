#define WINDOW_SDL
#include <poglib/application.h>

typedef struct content_t {

    gltexture2d_t texture;
    glshader_t shader;

} content_t ;

void coordinate_systems_multiple_init(application_t *app) 
{
    content_t cont = {
        .texture = gltexture2d_init("res/pepe_ez.png"),
        .shader = glshader_from_file_init("lib/poglib/res/shaders/simple3d.vs", "lib/poglib/res/shaders/simple3d.fs"),
    };

    application_pass_content(app, &cont);

    matrix4f_t projection = MATRIX4F_IDENTITY;
    projection = glms_perspective(glm_rad(45.0f), app->window.aspect_ratio, 0.1f, 100.0f);
    matrix4f_t view = MATRIX4F_IDENTITY;
    view       = glms_translate(view, (vec3f_t ){ 0.0f, 0.0f, -3.0f});

    glshader_send_uniform_matrix4f(
            &cont.shader,
            "projection",
            projection);
    matrix4f_print("Projection\n", projection);
    glshader_send_uniform_matrix4f(
            &cont.shader,
            "view",
            view);
    matrix4f_print("View\n", view);
}

void coordinate_systems_multiple_update(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    window_update_user_input(win);

}

void coordinate_systems_multiple_render(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    glrenderer3d_t rd3d = {
        .shader = &c->shader,
        .texture = &c->texture
    };
    vec3f_t cubePositions[] = {
        (vec3f_t ){ 0.0f,  0.0f,  0.0f},
        (vec3f_t ){ 2.0f,  5.0f, -15.0f},
        (vec3f_t ){-1.5f, -2.2f, -2.5f},
        (vec3f_t ){-3.8f, -2.0f, -12.3f},
        (vec3f_t ){ 2.4f, -0.4f, -3.5f},
        (vec3f_t ){-1.7f,  3.0f, -7.5f},
        (vec3f_t ){ 1.3f, -2.0f, -2.5f},
        (vec3f_t ){ 1.5f,  2.0f, -2.5f},
        (vec3f_t ){ 1.5f,  0.2f, -1.5f},
        (vec3f_t ){-1.3f,  1.0f, -1.5f}
    };


    for (unsigned int i = 0; i < 10; i++)
    {
        // calculate the model matrix for each object and pass it to shader before drawing
        matrix4f_t model = MATRIX4F_IDENTITY;
        model = glms_translate(model, cubePositions[i]);
        float angle = 20.0f * i;
        model = glms_rotate(model, glm_rad(angle), (vec3f_t ){1.0f, 0.3f, 0.5f});
        glshader_send_uniform_matrix4f(
                &c->shader,
                "model",
                model);
        glrenderer3d_draw_cube(&rd3d);
        /*matrix4f_print("Model\n", model);*/
    }
}

void coordinate_systems_multiple_destroy(application_t *app) 
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
            .title = "coordinate_systems_multiple",
            .width = 800,
            .height = 600,
            .aspect_ratio = (f32)800 / (f32)600
        },   
        .content = {
            .size = sizeof(content_t )
        },
        .init       = coordinate_systems_multiple_init,
        .update     = coordinate_systems_multiple_update,
        .render     = coordinate_systems_multiple_render,
        .destroy    = coordinate_systems_multiple_destroy
    };

    application_run(&app);

    return 0;
}

