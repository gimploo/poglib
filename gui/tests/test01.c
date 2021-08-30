#include "../crapgui.h"


void stuff(void *arg)
{
}

void empty(void *arg){}

int main(void)
{
    window_t window = window_init("TEST", 1080, 920, SDL_INIT_VIDEO);
    window_set_background(&window, (vec4f_t) {1.0f, 1.0f, 1.0f, 1.0f});

    gl_ascii_font_t font = gl_ascii_font_init("./../../res/ascii_fonts/charmap-oldschool_black.png", 18, 7);
    crapgui_t gui = crapgui_init(&window, &font);

    frame_t frame01 = frame_init(&gui, (vec2f_t ){-1.0f, 1.0f}, 1.0f, 1.0f);
    button_t button01 = button_init("button01", (vec2f_t ){-0.5f, 0.5f});
    slider_t slider01 = slider_init((vec2f_t ){0.0f, 5.0f}, (vec2f_t ){-0.5f, 0.2f});
    label_t label01 = label_init("label01", (vec2f_t ){-0.2f, 1.0f}, 0.1f);

    window_while_is_open(&window)
    {

        //TODO: Make all compoenets take in the frame instead of gui as argument and have frame take 
        //gui as argument instead. Since we would need take mouse position relative to the frame
        //instead of the window

        frame_begin(&frame01);
            
            if (button_is_mouse_over(&gui, &button01)) {
                button_update_color(&button01, ((vec3f_t){0.0f, 0.0f,1.0f}));
                button_update_label(&button01, "mouse over");
            }
            if (button_is_mouse_clicked(&gui, &button01)) {
                button_update_color(&button01, ((vec3f_t){0.0f, 1.0f,0.0f}));
                button_update_label(&button01, "mouse clicked");
            }
            if (button_is_mouse_dragging(&gui, &button01)) {
                button_update_color(&button01, ((vec3f_t){0.0f, 1.0f,0.0f}));
                button_update_label(&button01, "mouse drag");
            }
            button_draw(&gui, &button01);

            if (slider_box_is_mouse_dragging(&gui, &slider01))
                printf("sadge\n"); 
            slider_draw(&gui, &slider01);

            if(label_is_mouse_dragging(&gui, &label01))
                label_update_value(&label01, "Among us");
            label_draw(&gui, &label01);
    
        frame_end(&frame01);

        crapgui_begin(&gui);

            frame_draw(&frame01);

        crapgui_end(&gui);

    }

    crapgui_destroy(&gui);
    gl_ascii_font_destroy(&font);

    window_destroy(&window);

    return 0;
}
