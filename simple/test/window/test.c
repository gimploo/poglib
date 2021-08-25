#include "../../window.h"

void loop (void *arg)
{
}

int main(void)
{
    window_t window = window_init("TEST", 1080, 920, SDL_INIT_VIDEO);
    window_t *sub_window = window_sub_window_init(&window, "SUB", 1080/2, 920/2, SDL_INIT_VIDEO);

    window_set_background(&window, (vec4f_t) {1.0f, 0.0f, 0.0f, 1.0f});
    window_set_background(sub_window, (vec4f_t) {0.0f, 1.0f, 0.0f, 1.0f});

    while (window.is_open)
    {
        window_render(&window, loop, NULL);
    }

    window_destroy(&window);

    return 0;
}
