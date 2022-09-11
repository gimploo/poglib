
#define WINDOW_GLFW
#include <poglib/application.h>

typedef struct content_t {

    const char *text;
    glshader_t shader;
    gltexture2d_t texture;

} content_t ;

void opengl__init(application_t *app) 
{
    content_t cont = {
        .text = "Hello world\n",
        .shader = glshader_default_init(),
        .texture = gltexture2d_init("res/pepe_ez.png")
        
    };

    application_pass_content(app, &cont);
}

void opengl__update(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);
    window_update_user_input(win);
}

void opengl__render(application_t *app) 
{
    content_t *c = application_get_content(app);
    glrenderer2d_t rd2d = glrenderer2d(&c->shader, &c->texture);

    glquad_t quad = glquad(
            quadf(vec3f(0.0f), 1.0f, 1.0f),
            COLOR_WHITE, 
            quadf(vec3f(0.0f), 1.0f, 1.0f),
            0);
    glrenderer2d_draw_quad(&rd2d, quad);
}

void opengl__destroy(application_t *app) 
{
    content_t *c = application_get_content(app);
    glshader_destroy(&c->shader);
    gltexture2d_destroy(&c->texture);
}

int main(void)
{
    application_t app = {
        .window = {
            .title = "opengl",
            .width = 800,
            .height = 600,
            .aspect_ratio = (f32)800 / (f32)600
        },   
        .content = {
            .size = sizeof(content_t )
        },
        .init       = opengl__init,
        .update     = opengl__update,
        .render     = opengl__render,
        .destroy    = opengl__destroy
    };

    application_run(&app);

    return 0;
}

