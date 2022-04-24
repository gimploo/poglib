#include "../../crapgui.h"


int test01(void)
{
    window_t window = window_init("TEST", 1080, 920, SDL_INIT_VIDEO);
    window_set_background(&window, (vec4f_t) {0.0f, 0.0f, 1.0f, 1.0f});

    glfreetypefont_t ftfont = glfreetypefont_init("lib/res/ttf_fonts/Roboto-Medium.ttf", 40);

    crapgui_t gui = crapgui_init(&window, &ftfont);

    frame_t frame01 = frame_init(&gui, (vec3f_t ){-1.0f, 1.0f, 1.0f}, 1.0f, 1.0f);
    button_t button01 = button_init("button01", (vec3f_t ){-0.5f, 0.5f});
    slider_t slider01 = slider_init((vec2f_t ){0.0f, 5.0f}, (vec3f_t ){-0.5f, 0.2f});
    label_t label01 = label_init("label01", (vec3f_t ){-0.2f, 0.8f}, 0.1f);

    /*drop_down_list_t list01 = drop_down_list_init("YO",(vec3f_t ){0.3f, 0.0f}, 0.2f, 0.1f, GREEN_COLOR);*/
    /*drop_down_list_t list02 = drop_down_list_init("Whats",(vec3f_t ){0.3f, 0.0f}, 0.2f, 0.1f, BLUE_COLOR);*/
    /*drop_down_list_t list03 = drop_down_list_init("GOOD",(vec3f_t ){0.3f, 0.0f}, 0.2f, 0.1f, RED_COLOR);*/

    /*menu_bar_t menu_bar01 = menu_bar_init((vec3f_t ){-1.0f, 1.0f}, 3);*/
    /*menu_bar_push_drop_down_list(&menu_bar01, &list01);*/
    /*menu_bar_push_drop_down_list(&menu_bar01, &list02);*/
    /*menu_bar_push_drop_down_list(&menu_bar01, &list03);*/

    /*drop_down_list_push_button(&list01, "bruh");*/
    /*drop_down_list_push_button(&list01, "bruh");*/
    /*drop_down_list_push_button(&list01, "bruh");*/


    while(window.is_open)
    {
        window_update_user_input(&window);

        frame_begin(&frame01);
            
            button_draw(&gui, &button01);
            slider_draw(&gui, &slider01);
            label_draw(&gui, &label01);
            /*menu_bar_draw(&gui, &menu_bar01);*/
    
        frame_end(&frame01);

        crapgui_begin_ui(&gui);

            if (button_is_mouse_over(&frame01, &button01)) {
                printf("mouse_over\n"); 
                button_update_color(&button01, ((vec4f_t){0.0f, 0.0f,1.0f}));
                button_update_label(&button01, "mouse over");
            }
            if (button_is_mouse_clicked(&frame01, &button01)) {
                printf("mouse_clicked\n"); 
                button_update_color(&button01, ((vec4f_t){0.0f, 1.0f,0.0f}));
                button_update_label(&button01, "mouse clicked");
            }
            if (button_is_mouse_dragging(&frame01, &button01)) {
                printf("mouse_drag\n"); 
                button_update_color(&button01, ((vec4f_t){0.0f, 1.0f, 0.0f}));
                button_update_label(&button01, "mouse drag");
            }

            if (slider_box_is_mouse_dragging(&frame01, &slider01))
                printf("slider\n");

            /*menu_bar_is_clicked(&frame01, &menu_bar01);*/

            frame_draw(&frame01);

        crapgui_end_ui(&gui);

        SDL_Log("DT:    %f\n", window_get_dt(&window));
        SDL_Log("FPS:   %f\n", window_get_fps(&window));

    }

    crapgui_destroy(&gui);
    glfreetypefont_destroy(&ftfont);

    window_destroy(&window);

    return 0;
}

int main(void)
{
    dbg_init();
    printf("TEST01\n");
    test01();
    dbg_destroy();

    return 0;
}
