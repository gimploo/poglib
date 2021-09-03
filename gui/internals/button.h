#pragma once

#include "./common.h"
#include "./frame.h"

button_t button_init(const char *label, vec2f_t norm_position)
{
    return (button_t ){
        .label = label,
        .norm_position = norm_position,
        .norm_width = BUTTON_DEFAULT_WIDTH,
        .norm_height = BUTTON_DEFAULT_HEIGHT,
        .norm_color = BUTTON_DEFAULT_COLOR,
        .__quad_vertices = quadf_init(norm_position, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT),
    };
}

#define button_update_color(pbutton, rgb) (pbutton)->norm_color = rgb
#define button_update_label(pbutton, text) (pbutton)->label = text

#define button_update_norm_position(pbutton, vec2) do {\
    (pbutton)->norm_position    = vec2;\
    (pbutton)->__quad_vertices  = quadf_init(vec2, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);\
} while(0)

void button_draw(crapgui_t *gui, button_t *button) 
{
    const gl_quad_t quad= gl_quad(button->__quad_vertices, button->norm_color, quadf(0));
    gl_renderer2d_draw_quad(&gui->renderer_handler, quad); 
    gl_ascii_font_render_text(
            gui->font_handler, 
            button->label, 
            vec2f_add(
                button->norm_position, 
                (vec2f_t ){ 0, (-button->norm_height/2)}), 
            button->norm_height/6);
}


bool button_is_mouse_over(frame_t *frame, button_t *button)
{
    //NOTE: this checks if the mouse is over the frame 
    if (!__frame_is_mouse_over(frame)) return false;

    button->norm_color = BUTTON_DEFAULT_COLOR;
    vec2f_t mouse_position = __frame_get_mouse_position_relative_to_frame_coordinate_axis(frame);
    return quadf_is_point_in_quad(button->__quad_vertices, mouse_position);
}

bool button_is_mouse_clicked(frame_t *frame, button_t *button)
{
    return (window_mouse_button_just_pressed(frame->gui_handler->window_handler) && button_is_mouse_over(frame, button));
}

bool button_is_mouse_dragging(frame_t *frame, button_t *button)
{
    // NOTE: this code will have the button rendererd as the mouse at the center
    bool is_drag = (window_mouse_button_is_held(frame->gui_handler->window_handler) && button_is_mouse_over(frame, button));

    if (is_drag) 
    {
        vec2f_t mouse_position = __frame_get_mouse_position_relative_to_frame_coordinate_axis(frame);
        vec2f_t mouse_at_center_offset_position = {
            .cmp[X] = (mouse_position.cmp[X] - (BUTTON_DEFAULT_WIDTH/2)),
            .cmp[Y] = (mouse_position.cmp[Y] + (BUTTON_DEFAULT_HEIGHT/2))
        };
        button_update_norm_position(button, mouse_at_center_offset_position);
    } 
    return is_drag;
}

