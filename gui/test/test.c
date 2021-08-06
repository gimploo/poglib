#include <stdio.h>
#include <stdlib.h>

#include "../crapui.h"



void loop(void *arg)
{
    button_t **buttons = (button_t **)arg;

    for (int i = 0; i < 4; i++) button_draw(buttons[i]);
        
}

int main(void)
{
    Window window = window_init(1080, 920, SDL_INIT_VIDEO);

    button_t button01 = button_init(
                "button01", 
                (vec4f) {0.4f, 0.3f, 0.0f, 1.0f}, 
                (vec2f) {-1.0f, +1.0f}, 
                0.5f, 0.5f
            );

    button_t button02 = button_init(
                "button02", 
                (vec4f) {0.3f, 0.0f, 0.0f, 1.0f}, 
                (vec2f) {0.0f, 1.0f}, 
                0.5f, 0.5f
            );
    button_t button03 = button_init(
                "button03", 
                (vec4f) {1.0f, 0.0f, 0.4f, 1.0f}, 
                (vec2f) {0.0f, 0.0f}, 
                0.5f, 0.5f
            );

    button_t button04 = button_init(
                "button04", 
                (vec4f) {0.0f, 1.0f, 0.3f, 1.0f}, 
                (vec2f) {-1.0f, +0.0f}, 
                0.5f, 0.5f
            );

    button_t* buttons[4] = {&button01, &button02, &button03, &button04};


    while (window.is_open)
    {
        window_process_user_input(&window);
        window_render(&window, loop, buttons); 
    }



    window_destroy(&window);

    return 0;
}
