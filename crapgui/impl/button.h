#pragma once
#include "../decl.h"
#include "./uielem.h"
#include "./frame.h"

bool __button_check_is_mouse_over_button(button_t *button)
{
    uielem_t *ui = button->__ui;
    frame_t *frame = ui->frame;
    window_t *win = window_get_current_active_window();
    vec2f_t mouse_position = __frame_get_mouse_position(frame);
    printf(VEC2F_FMT "\n", VEC2F_ARG(&mouse_position));
    if(quadf_is_point_in_quad(ui->vertices, mouse_position)) {
        button->is_hot = true;
        button->is_active = false;
    } else {
        button->is_hot = false;
        button->is_active = false;
    }
    ui->is_changed = true;
    return button->is_hot;
}

bool __button_check_is_mouse_clicked_button(button_t *button)
{
    window_t *win = window_get_current_active_window();

    uielem_t *ui = button->__ui;
    if (window_mouse_button_just_pressed(win) && __button_check_is_mouse_over_button(button)) {
        button->is_active   = true;
    } else {
        button->is_active = false;
    }

    return button->is_active;
}


bool __button_check_is_mouse_dragging_button(button_t *button)
{
    window_t *win = window_get_current_active_window();
    bool is_drag = (window_mouse_button_is_held(win) && __button_check_is_mouse_over_button(button));
    uielem_t *ui = button->__ui;

    if (is_drag) 
    {
        const vec2f_t dim = DEFAULT_BUTTON_DIMENSIONS;
        vec2f_t mouse_position = window_mouse_get_norm_position(win);
        vec2f_t mouse_at_center_offset_position = {
            .cmp[X] = (mouse_position.cmp[X] - (dim.cmp[X]/2)),
            .cmp[Y] = (mouse_position.cmp[Y] + (dim.cmp[Y]/2))
        };

        ui->pos = mouse_at_center_offset_position;
    } 
    
    return is_drag;
}


void __button_update(uielem_t *uielem)
{
    assert(uielem);
    assert(uielem->type == UI_BUTTON);

    button_t *button = &uielem->button;

    __button_check_is_mouse_over_button(button);

    if (button->is_hot)
        uielem->color = button->hover_color;
    else 
        uielem->color = button->base_color;

}


button_t button_init(uielem_t *ui) 
{
    button_t button = {
        .base_color     = DEFAULT_BUTTON_COLOR,
        .hover_color    = DEFAULT_BUTTON_HOVER_COLOR,
        .is_active      = false,
        .is_hot         = false,
        .__ui           = ui
    };

    return button;
}

