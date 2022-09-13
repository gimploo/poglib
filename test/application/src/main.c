#define WINDOW_GLFW
#include <poglib/application.h>

typedef struct content_t {

    const char *text;

} content_t ;

void application_init(application_t *app) 
{
    content_t cont = {
        .text = "Hello world\n" 
    };

    application_pass_content(app, &cont);
}

void application_update(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    window_update_user_input(win);

    printf("FPS: %f\n", application_get_fps(app));
    printf("DT:  %f\n", application_get_dt(app));
}

void application_render(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);
}

void application_destroy(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);
}

int main(void)
{
    application_t app = {
        .window = {
            .title = "application",
            .width = 800,
            .height = 600,
            .aspect_ratio = (f32)800 / (f32)600,
            .fps_limit = 60
        },   
        .content = {
            .size = sizeof(content_t )
        },
        .init       = application_init,
        .update     = application_update,
        .render     = application_render,
        .destroy    = application_destroy
    };

    application_run(&app);

    return 0;
}

