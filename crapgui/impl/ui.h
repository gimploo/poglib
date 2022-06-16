#pragma once
#include "../decl.h"


void __ui_destroy(ui_t *elem)
{
    gltext_destroy(&elem->__cache.texts);
}

void __ui_cache_vertices(ui_t *ui, const crapgui_t *gui)
{
    ui->__cache.quad = 
        quadf(vec3f(ui->pos), 
                ui->styles.width, 
                ui->styles.height);

    glbatch_t *txtbatch = &ui->__cache.texts.text;

    vec2f_t centerpos = {
        .cmp[X] = ui->__cache.quad.vertex[BOTTOM_LEFT].cmp[X],
        .cmp[Y] = ui->__cache.quad.vertex[BOTTOM_LEFT].cmp[Y] //- ui->styles.height/2
    };

    const glfreetypefont_t *font = &gui->ui_assets.fonts[ui->type];
    glbatch_clear(txtbatch);
    glfreetypefont_add_text_to_batch(
            font, 
            txtbatch, 
            ui->title, 
            centerpos, 
            ui->styles.textcolor);

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

void __ui_edit_mode_is_mouse_over(ui_t *ui, const frame_t *frame, crapgui_t *gui)
{
    if (gui->edit_mode.active_ui) return;

    const vec2f_t mousepos  = window_mouse_get_norm_position(global_window);
    const quadf_t rect      = quadf(
                                vec3f(ui->pos), 
                                ui->styles.width, 
                                ui->styles.height); 
    window_t *win = gui->win;

    if(window_mouse_button_is_held(win) 
        && quadf_is_point_in_quad(rect, mousepos))
    {
        gui->edit_mode.active_ui = ui;

    } else {
        printf("ui is null\n");
        gui->edit_mode.active_ui = NULL;
    }
}

void __ui_edit_mode_is_mouse_held(const frame_t *frame, const crapgui_t *gui)
{
    const vec2f_t mousepos  = window_mouse_get_norm_position(global_window);
    const window_t *win     = window_get_current_active_window();

    if (gui->edit_mode.active_ui 
        && window_mouse_button_is_held(global_window))
    {
        ui_t *ui = gui->edit_mode.active_ui;
        const vec2f_t dim = { 
            ui->styles.width / 2.0f, 
            ui->styles.height / 2.0f 
        };

        ui->pos = (vec2f_t ){
            mousepos.cmp[X] - dim.cmp[X],
            mousepos.cmp[Y] + dim.cmp[Y],
        };
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
    if (gui->edit_mode.is_on) {

        ui->is_hot = ui->is_active = false;
        __ui_edit_mode_is_mouse_over(ui, frame, (crapgui_t *)gui);
        __ui_edit_mode_is_mouse_held(frame, gui);

    } else 
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
