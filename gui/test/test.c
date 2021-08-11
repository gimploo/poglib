#include <stdio.h>
#include <stdlib.h>

#include "../crapui.h"


void loop(void *arg)
{
    window_t *window = (window_t *)arg;

    crapgui_t gui = gui_init(window);
    /*gui.font = gl_ascii_font_init("./glyph_atlas.png", 16, 6);*/

    gui_begin(&gui);
    {
        button_t button01 = button_begin(&gui, "button01");
        {
             if (button_is_pressed(&gui, &button01)) {

                printf("%s is clicked\n", button01.label);

             } else if (button_is_dragged(&gui, &button01)) {

                vec2f_t new_pos = gui.sub_window->mouse_handler.position;
                button_set_position(&gui, &button01, new_pos);
                printf("%s is dragged\n", button01.label);

            } else if  (button_is_mouseover(&gui, &button01)) {
                
                button_set_color(&gui, &button01, (vec4f_t) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button01);

        button_t button02 = button_begin(&gui, "button02");
        {
             if (button_is_pressed(&gui, &button02)) {

                printf("%s is clicked\n", button02.label);

             } else if (button_is_dragged(&gui, &button02)) {

                vec2f_t new_pos = gui.sub_window->mouse_handler.position;
                button_set_position(&gui, &button02, new_pos);
                printf("%s is dragged\n", button02.label);

            } else if  (button_is_mouseover(&gui, &button02)) {
                
                button_set_color(&gui, &button02, (vec4f_t) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button02);

        button_t button03 = button_begin(&gui, "button03");
        {
             if (button_is_pressed(&gui, &button03)) {

                printf("%s is clicked\n", button03.label);

             } else if (button_is_dragged(&gui, &button03)) {

                vec2f_t new_pos = gui.sub_window->mouse_handler.position;
                button_set_position(&gui, &button03, new_pos);
                printf("%s is dragged\n", button03.label);

            } else if  (button_is_mouseover(&gui, &button03)) {
                
                button_set_color(&gui, &button03, (vec4f_t) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button03);

        button_t button04 = button_begin(&gui, "button04");
        {
             if (button_is_pressed(&gui, &button04)) {

                printf("%s is clicked\n", button04.label);

             } else if (button_is_dragged(&gui, &button04)) {

                vec2f_t new_pos = gui.sub_window->mouse_handler.position;
                button_set_position(&gui, &button04, new_pos);
                printf("%s is dragged\n", button04.label);

            } else if  (button_is_mouseover(&gui, &button04)) {
                
                button_set_color(&gui, &button04, (vec4f_t) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button04);

        button_t button05 = button_begin(&gui, "button05"); {
             if (button_is_pressed(&gui, &button05)) {

                printf("%s is clicked\n", button05.label);

             } else if (button_is_dragged(&gui, &button05)) {

                vec2f_t new_pos = gui.sub_window->mouse_handler.position;
                button_set_position(&gui, &button05, new_pos);
                printf("%s is dragged\n", button05.label);

            } else if  (button_is_mouseover(&gui, &button05)) {
                
                button_set_color(&gui, &button05, (vec4f_t) {1.0f, 0.0f, 0.0f, 1.0f});
            }
        }
        button_end(&button05);
    }
    gui_end(&gui);
}

int main(void)
{
    window_t window = window_init(1080, 920, SDL_INIT_VIDEO);

    while (window.is_open)
    {
        window_process_user_input(&window);
        window_render(&window, loop, &window); 
    }


    window_destroy(&window);

    return 0;
}

