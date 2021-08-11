#include "../../gl_renderer.h"
#include "../../window.h"

void loop(void *arg)
{
    (void)arg;
    printf("hello world\n");
}


int main(void)
{
    window_t window = window_init(1080, 920, SDL_INIT_VIDEO);
    window_set_background(&window, (vec4f) {1.0f, 0.0f, 0.0f, 1.0f});

    while (window.is_open)
    {
        window_process_user_input(&window);
        window_render(&window, loop, NULL);
    }

    return 0;
}
