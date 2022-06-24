#include <poglib/application.h>

void app_init(application_t *app) { }
void app_render(application_t *app) { }
void app_destroy(application_t *app) { }

void app_update(application_t *app)
{
    window_t *win = application_get_window(app);
    if (window_mouse_button_is_pressed(win, SDL_MOUSEBUTTON_LEFT))
            printf("LEFT PRESSED");
    else if (window_mouse_button_is_pressed(win, SDL_MOUSEBUTTON_RIGHT))
            printf("RIGHT PRESSED");
    else if (window_mouse_button_is_pressed(win, SDL_MOUSEBUTTON_MIDDLE))
            printf("MIDDLE PRESSED");
    else
        printf("...\n");

    if (window_mouse_button_is_released(win, SDL_MOUSEBUTTON_LEFT))
            printf("LEFT RELEASED");
    else if (window_mouse_button_is_released(win, SDL_MOUSEBUTTON_RIGHT))
            printf("RIGHT RELEASED");
    else if (window_mouse_button_is_released(win, SDL_MOUSEBUTTON_MIDDLE))
            printf("MIDDLE RELEASED");
    else
        printf("...\n");
}


int main(void)
{
    application_t app = {0};
    app = (application_t ){
        .window_height = 400,
        .window_width = 300,
        .window_title = "TEST",

        .init = app_init,
        .update = app_update,
        .render = app_render,
        .destroy = app_destroy
    };

    application_run(&app);

    return 0;
}
