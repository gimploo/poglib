#pragma once
#include "../decl.h"


void __ui_destroy(ui_t *elem)
{
    gltext_destroy(&elem->__cache.texts);
}

void __ui_cache_vertices(ui_t *ui, const crapgui_t *gui)
{
    ui->__cache.quad = 
        quadf(vec3f(ui->pos), ui->styles.width, ui->styles.height);

    glbatch_t *txtbatch = &ui->__cache.texts.text;

    vec2f_t centerpos = {
        .cmp[X] = ui->pos.cmp[X] ,
        .cmp[Y] = ui->pos.cmp[Y] - ui->styles.height/2
    };

    glfreetypefont_t *font = crapgui_get_font(gui, ui->type);
    glbatch_clear(txtbatch);
    glfreetypefont_add_text_to_batch(
            font, txtbatch, ui->title, centerpos, ui->styles.textcolor);

}


void __ui_check_is_mouse_over(ui_t *ui, const frame_t *frame)
{
    window_t *win = window_get_current_active_window();
    vec2f_t mouse_position = frame_get_mouse_position(frame);

    if(quadf_is_point_in_quad(ui->__cache.quad, mouse_position)) {
        ui->is_hot      = true;
        ui->is_active   = false;
    } else {
        ui->is_hot      = false;
        ui->is_active   = false;
    }

}

void __ui_check_is_mouse_clicked(ui_t *ui, const crapgui_t *gui)
{
    window_t *win = gui->win;

    if (window_mouse_button_just_pressed(win) && ui->is_hot)
        ui->is_active   = true;
}

void __ui_update(ui_t *ui, const frame_t *frame, const crapgui_t *gui)
{
    assert(ui->type != UI_FRAME);

    __ui_check_is_mouse_over(ui, frame);
    __ui_cache_vertices(ui, gui);

    switch(ui->type)
    {
        case UI_BUTTON:

            __ui_check_is_mouse_clicked(ui, gui);

            if (ui->is_hot)
                ui->__cache.glquad = glquad(ui->__cache.quad,
                                    ui->styles.hovercolor,
                                    quadf(vec3f(0.0f), 0.0f, 0.0f), 0);
            else
                ui->__cache.glquad = glquad(ui->__cache.quad,
                                    ui->styles.color,
                                    quadf(vec3f(0.0f), 0.0f, 0.0f), 0);
        break;

        case UI_LABEL:
            ui->__cache.glquad = 
                glquad(ui->__cache.quad, ui->styles.color,
                        quadf(vec3f(0.0f), 0.0f, 0.0f), 0);
        break;

        default: eprint("type not accounted for ");
    }

}
