#include "../../gl_renderer.h"
#include "../../window.h"

void loop(void *arg)
{
    /*window_t *sub_window = (window_t *)arg;*/
    /*window_process_user_input(sub_window);*/
    /*window_render(sub_window, NULL, NULL);*/
    (void) arg;
    printf("hello world\n");
}


int main(void)
{
    window_t window = window_init(1080, 920, SDL_INIT_VIDEO);
    /*window_t sub_window = window_init(1080/2, 920/2, SDL_INIT_VIDEO);*/

    window_set_background(&window, (vec4f_t) {1.0f, 0.0f, 0.0f, 1.0f});
    /*window_set_background(&sub_window, (vec4f_t) {0.0f, 1.0f, 0.0f, 1.0f});*/

    while (window.is_open)
    {
        window_render(&window, loop,NULL);// &sub_window);
    }

    /*window_destroy(&sub_window);*/
    window_destroy(&window);

    return 0;
}
