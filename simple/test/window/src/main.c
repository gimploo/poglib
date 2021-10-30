
#include "../../../window.h"

void stuff(void *args)
{
    (void)args;
}

int main(void)
{
    window_t window = window_init("test", 700, 600, SDL_INIT_VIDEO);

    vec4f_t color = {0};
    window_while_is_open(&window)
    {
        window_cap_fps(&window, 30);

        color = vec4f_add(color, (vec4f_t){0.1f, 0.1f, 0.1f, 0.0f});

        window_set_background(&window, color);
        window_render_stuff(&window, stuff, NULL);

        /*SDL_Log("FPS: %f\n", window_get_fps(&window));*/
        SDL_Log("DT: %f\n\n", window_get_dt(&window));
    }

    window_destroy(&window);

    return 0;
}
