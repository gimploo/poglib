#include <stdio.h>
#include <stdlib.h>

#include "../crapui.h"


void loop(void *arg)
{
    Window *window = (Window *)arg;

    crapgui_t gui = gui_init(window);

    gui_begin(&gui);
    {
        button_t button01 = button_begin(&gui, "button01");
        {
             if (button_is_pressed(&button01)) {

                printf("%s is clicked\n", button01.label);

             } else if (button_is_dragged(&button01)) {

                vec2f new_pos = button01.gui_handler->window->mouse_handler.position;
                button_set_position(&button01, new_pos);
                printf("%s is dragged\n", button01.label);

            } else if  (button_is_mouseover(&button01)) {
                
                button_set_color(&button01, (vec4f) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button01);

        button_t button02 = button_begin(&gui, "button02");
        {
             if (button_is_pressed(&button02)) {

                printf("%s is clicked\n", button02.label);

             } else if (button_is_dragged(&button02)) {

                vec2f new_pos = button02.gui_handler->window->mouse_handler.position;
                button_set_position(&button02, new_pos);
                printf("%s is dragged\n", button02.label);

            } else if  (button_is_mouseover(&button02)) {
                
                button_set_color(&button02, (vec4f) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button02);

        button_t button03 = button_begin(&gui, "button03");
        {
             if (button_is_pressed(&button03)) {

                printf("%s is clicked\n", button03.label);

             } else if (button_is_dragged(&button03)) {

                vec2f new_pos = button03.gui_handler->window->mouse_handler.position;
                button_set_position(&button03, new_pos);
                printf("%s is dragged\n", button03.label);

            } else if  (button_is_mouseover(&button03)) {
                
                button_set_color(&button03, (vec4f) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button03);

        button_t button04 = button_begin(&gui, "button04");
        {
             if (button_is_pressed(&button04)) {

                printf("%s is clicked\n", button04.label);

             } else if (button_is_dragged(&button04)) {

                vec2f new_pos = button04.gui_handler->window->mouse_handler.position;
                button_set_position(&button04, new_pos);
                printf("%s is dragged\n", button04.label);

            } else if  (button_is_mouseover(&button04)) {
                
                button_set_color(&button04, (vec4f) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button04);

        button_t button05 = button_begin(&gui, "button05"); {
             if (button_is_pressed(&button05)) {

                printf("%s is clicked\n", button05.label);

             } else if (button_is_dragged(&button05)) {

                vec2f new_pos = button05.gui_handler->window->mouse_handler.position;
                button_set_position(&button05, new_pos);
                printf("%s is dragged\n", button05.label);

            } else if  (button_is_mouseover(&button05)) {
                
                button_set_color(&button05, (vec4f) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button05);
    }
    gui_end(&gui);
}

int main(void)
{
    Window window = window_init(1080, 920, SDL_INIT_VIDEO);

    while (window.is_open)
    {
        window_process_user_input(&window);
        window_render(&window, loop, &window); 
    }


    window_destroy(&window);

    return 0;
}

