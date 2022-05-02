#pragma once
#include "../decl.h"


void __ui_destroy(ui_t *elem)
{
    elem->font = NULL;
    glbatch_destroy(&elem->__textbatch);
}

void __ui_cache_vertices(ui_t *ui)
{
    ui->__vertices = 
        quadf(vec3f(ui->pos), ui->dim.cmp[X], ui->dim.cmp[Y]);

    glbatch_t *txtbatch = &ui->__textbatch;

    vec2f_t centerpos = {
        .cmp[X] = ui->pos.cmp[X] ,
        .cmp[Y] = ui->pos.cmp[Y] - ui->dim.cmp[Y]/2
    };

    glfreetypefont_t *font = ui->font;
    glbatch_clear(txtbatch);
    glfreetypefont_add_text_to_batch(
            font, txtbatch, ui->title, centerpos, ui->textcolor);

}


void __ui_check_is_mouse_over(ui_t *ui, const frame_t *frame)
{
    window_t *win = window_get_current_active_window();
    vec2f_t mouse_position = frame_get_mouse_position(frame);

    ui->__is_changed = true;

    if(quadf_is_point_in_quad(ui->__vertices, mouse_position)) {

        ui->is_hot = true;
        ui->is_active = false;

    } else {

        ui->is_hot = false;
        ui->is_active = false;
    }

}

void __ui_check_is_mouse_clicked(ui_t *ui, const frame_t *frame)
{
    window_t *win = frame->gui->win;

    if (window_mouse_button_just_pressed(win) && ui->is_hot)
        ui->is_active   = true;
    else 
        ui->is_active = false;
}

void __ui_update(ui_t *ui, const frame_t *frame)
{
    assert(ui->type != UI_FRAME);

    __ui_check_is_mouse_over(ui, frame);

    if (!ui->__is_changed) return;

    printf("Updating ui %s \n", ui->title);

    __ui_cache_vertices(ui);

    switch(ui->type)
    {
        case UI_BUTTON:

            __ui_check_is_mouse_clicked(ui, frame);

            if (ui->is_hot)
                ui->__glvertices = glquad(ui->__vertices,
                                    ui->hovercolor,
                                    quadf(vec3f(0.0f), 0.0f, 0.0f), 0);
            else
                ui->__glvertices = glquad(ui->__vertices,
                                    ui->basecolor,
                                    quadf(vec3f(0.0f), 0.0f, 0.0f), 0);
        break;

        case UI_LABEL:
            ui->__glvertices = 
                glquad(ui->__vertices, ui->basecolor,
                        quadf(vec3f(0.0f), 0.0f, 0.0f), 0);
        break;

        default: eprint("type not accounted for ");
    }


    ui->__is_changed = false;
}
