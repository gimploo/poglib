#pragma once
#include "../decl.h"
#include "./uielem.h"

bool __button_check_is_mouse_over_button(const crapgui_t *gui, button_t *button)
{
    frame_t *frame = gui->active_frame;
    if (!frame) return false;

    uielem_t *ui = button->__ui;

    vec2f_t mouse_position = frame->__relative_mouse_pos;
    if(quadf_is_point_in_quad(button->__vertices, mouse_position)) {
        button->is_hot = true;
        button->is_active = false;
    } 
    return ui->is_changed;
}

bool __button_check_is_mouse_clicked_button(const crapgui_t *gui, button_t *button)
{
    window_t *win = gui->win;
    frame_t *frame = gui->active_frame;
    if (!frame) return false;

    uielem_t *ui = button->__ui;
    if (window_mouse_button_just_pressed(win) && __button_check_is_mouse_over_button(gui, button)) {
        button->is_active   = true;
    } else {
        button->is_active = false;
    }

    return button->is_active;
}


bool __button_check_is_mouse_dragging_button(const crapgui_t *gui, button_t *button)
{
    frame_t *frame = gui->active_frame;
    if (!frame) return false;

    window_t *win = gui->win;
    bool is_drag = (window_mouse_button_is_held(win) && __button_check_is_mouse_over_button(gui, button));
    uielem_t *ui = button->__ui;

    if (is_drag) 
    {
        const vec2f_t dim = DEFAULT_BUTTON_DIMENSIONS;
        vec2f_t mouse_position = frame->__relative_mouse_pos;
        vec2f_t mouse_at_center_offset_position = {
            .cmp[X] = (mouse_position.cmp[X] - (dim.cmp[X]/2)),
            .cmp[Y] = (mouse_position.cmp[Y] + (dim.cmp[Y]/2))
        };

        ui->pos = mouse_at_center_offset_position;
    } 
    
    return is_drag;
}


void __button_update(uielem_t *uielem, const crapgui_t *gui)
{
    assert(uielem);
    assert(uielem->type == UI_BUTTON);

    button_t *button = &uielem->button;

    button->__vertices = 
        quadf(vec3f(uielem->pos), uielem->dim.cmp[X], uielem->dim.cmp[Y]);

    uielem->glvertices = 
        glquad(button->__vertices,
                button->is_hot ? button->hover_color : uielem->color,
                quadf(vec3f(0.0f), 0.0f, 0.0f), 0);

    vec2f_t centerpos = {
        .cmp[X] = uielem->pos.cmp[X] + uielem->dim.cmp[X] / 2,
        .cmp[Y] = uielem->pos.cmp[Y] - uielem->dim.cmp[Y]
    };

    glfreetypefont_t *font = uielem->font;
    glbatch_t *txtbatch = &uielem->textbatch;
    glbatch_clear(txtbatch);
    glfreetypefont_add_text_to_batch(
            font, txtbatch, uielem->title, centerpos, COLOR_WHITE);
}


button_t button_init(uielem_t *ui) 
{
    button_t button = {
        .hover_color    = DEFAULT_BUTTON_COLOR,
        .is_active      = false,
        .is_hot         = false,
        .__ui           = ui
    };

    return button;
}

