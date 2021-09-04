#include "../crapgui.h"
#include "./../../color.h"

void stuff(void *arg)
{
}

void empty(void *arg){}

int main(void)
{
    window_t window = window_init("TEST", 1080, 920, SDL_INIT_VIDEO);
    window_set_background(&window, (vec4f_t) {0.0f, 0.0f, 1.0f, 1.0f});

    gl_ascii_font_t font = gl_ascii_font_init("./../../res/ascii_fonts/charmap-oldschool_black.png", 18, 7);
    crapgui_t gui = crapgui_init(&window, &font);

    frame_t frame01 = frame_init(&gui, (vec2f_t ){-1.0f, 1.0f}, 2.0f, 2.0f);
    button_t button01 = button_init("button01", (vec2f_t ){-0.5f, 0.5f});
    slider_t slider01 = slider_init((vec2f_t ){0.0f, 5.0f}, (vec2f_t ){-0.5f, 0.2f});
    /*label_t label01 = label_init("label01", (vec2f_t ){-0.2f, 1.0f}, 0.1f);*/

    drop_down_list_t list01 = drop_down_list_init("YO",(vec2f_t ){0.3f, 0.0f}, 0.2f, 0.1f, GREEN_COLOR);
    drop_down_list_t list02 = drop_down_list_init("Whats",(vec2f_t ){0.3f, 0.0f}, 0.2f, 0.1f, BLUE_COLOR);
    drop_down_list_t list03 = drop_down_list_init("GOOD",(vec2f_t ){0.3f, 0.0f}, 0.2f, 0.1f, RED_COLOR);

    menu_bar_t menu_bar01 = menu_bar_init((vec2f_t ){-1.0f, 1.0f}, 3);
    menu_bar_push_drop_down_list(&menu_bar01, &list01);
    menu_bar_push_drop_down_list(&menu_bar01, &list02);
    menu_bar_push_drop_down_list(&menu_bar01, &list03);


    window_while_is_open(&window)
    {
        frame_begin(&frame01);
            
            button_draw(&gui, &button01);
            slider_draw(&gui, &slider01);
            /*label_draw(&gui, &label01);*/
            /*drop_down_list_draw(&gui, &list01);*/
            menu_bar_draw(&gui, &menu_bar01);
    
        frame_end(&frame01);

        crapgui_begin_ui(&gui);

            if (button_is_mouse_over(&frame01, &button01)) {
                printf("mouse_over\n"); 
                button_update_color(&button01, ((vec3f_t){0.0f, 0.0f,1.0f}));
                button_update_label(&button01, "mouse over");
            }
            if (button_is_mouse_clicked(&frame01, &button01)) {
                printf("mouse_clicked\n"); 
                button_update_color(&button01, ((vec3f_t){0.0f, 1.0f,0.0f}));
                button_update_label(&button01, "mouse clicked");
            }
            if (button_is_mouse_dragging(&frame01, &button01)) {
                printf("mouse_drag\n"); 
                button_update_color(&button01, ((vec3f_t){0.0f, 1.0f, 0.0f}));
                button_update_label(&button01, "mouse drag");
            }

            if (slider_box_is_mouse_dragging(&frame01, &slider01))
                printf("slider\n");

            /*drop_down_list_is_clicked(&frame01, &list01);*/

            menu_bar_is_clicked(&frame01, &menu_bar01);

            frame_draw(&frame01);

        crapgui_end_ui(&gui);

    }

    crapgui_destroy(&gui);
    gl_ascii_font_destroy(&font);

    window_destroy(&window);

    return 0;
}
