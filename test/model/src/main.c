#define WINDOW_SDL
#include <poglib/application.h>
#include <poglib/util/glcamera.h>

#define WINDOW_WIDTH    600
#define WINDOW_HEIGHT   800

typedef struct content_t {

    glmodel_t       model;
    glshader_t      shader;
    gltexture2d_t   texture;
    glcamera_t      camera;

} content_t ;

void model_init(application_t *app) 
{
    content_t cont = {
        .model = glmodel_init("res/backpack.obj"),
        .shader = glshader_from_file_init("res/shader.vs", "res/shader.fs"),
        .texture = gltexture2d_init("res/diffuse.jpg"),
        .camera = glcamera_perspective((vec3f_t ) { 0.0f, 0.0f, 8.0f }),
    };

    application_pass_content(app, &cont);
}

void model_update(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    window_update_user_input(win);
    glcamera_process_input(&c->camera, application_get_dt(app));

    matrix4f_t model = MATRIX4F_IDENTITY;
    glshader_send_uniform_matrix4f(&c->shader, "model", model);

    matrix4f_t projection = MATRIX4F_IDENTITY;
    projection = glms_perspective(
                    radians(c->camera.zoom), app->window.aspect_ratio, 0.1f, 100.0f);
    glshader_send_uniform_matrix4f(&c->shader, "projection", projection);

    matrix4f_t view = MATRIX4F_IDENTITY;
    glshader_send_uniform_matrix4f(
            &c->shader, "view", 
            glcamera_getview(&c->camera));
}

void model_render(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    glrenderer3d_draw_model(
            &(glrenderer3d_t ) {
                .shader = &c->shader,
                .texture = &c->texture
            }, &c->model
    );
}

void model_destroy(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    glmodel_destroy(&c->model);
    glshader_destroy(&c->shader);
    gltexture2d_destroy(&c->texture);
}

int main(void)
{
    application_t app = {
        .window = {
            .title = "model",
            .width = WINDOW_WIDTH,
            .height = WINDOW_HEIGHT,
            .aspect_ratio = (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT,
            .fps_limit = 60,
            .background_color = COLOR_WHITE
        },   
        .content = {
            .size = sizeof(content_t )
        },
        .init       = model_init,
        .update     = model_update,
        .render     = model_render,
        .destroy    = model_destroy
    };

    application_run(&app);

    return 0;
}

