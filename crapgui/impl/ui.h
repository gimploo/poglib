#pragma once
#include "../decl.h"
void __ui_destroy(ui_t *elem)
{
    gltext_destroy(&elem->__cache.texts);
}

void __ui_cache_vertices(ui_t *ui, const crapgui_t *gui)
{
    ui->__cache.quad = 
        quadf((vec3f_t ){ui->pos.x, ui->pos.y, 0.f}, 
                ui->styles.width, 
                ui->styles.height);

    glbatch_t *txtbatch = &ui->__cache.texts.text;

    vec2f_t centerpos = {
        .x = ui->__cache.quad.vertex[BOTTOM_LEFT].x,
        .y = ui->__cache.quad.vertex[BOTTOM_LEFT].y //- ui->styles.height/2
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


void __ui_is_mouse_over(ui_t *ui, const frame_t *frame, crapgui_t *gui)
{
    window_t *win = window_get_current_active_window();
    vec2f_t mouse_position = frame_get_mouse_position(frame);

    if(quadf_is_point_in_quad(ui->__cache.quad, mouse_position)) {
        gui->currently_active.ui = ui;
        ui->is_hot      = true;
        ui->is_active   = false;
    } else {
        ui->is_hot      = false;
        ui->is_active   = false;
    }

}


void __ui_is_mouse_clicked(ui_t *ui, const crapgui_t *gui)
{
    window_t *win = gui->win;

    if (window_mouse_button_just_pressed(win, SDL_MOUSEBUTTON_LEFT) 
        && ui->is_hot) 
    {
        ui->is_active   = true;
    }
}

void __ui_update(ui_t *ui, frame_t *frame, crapgui_t *gui)
{
    /*printf("Recaching ui %s\n", ui->title);*/
    if (ui->__cache.cache_again)
        __ui_cache_vertices(ui, gui);

    switch(ui->type)
    {
        case UI_BUTTON:
            __ui_is_mouse_over(ui, frame, gui);
            __ui_is_mouse_clicked(ui, gui);

            if (ui->is_hot)
                ui->__cache.glquad = glquad(ui->__cache.quad,
                                    ui->styles.hovercolor,
                                    quadf(vec3f(0.0f), 0.0f, 0.0f));
            else
                ui->__cache.glquad = glquad(ui->__cache.quad,
                                    ui->styles.color,
                                    quadf(vec3f(0.0f), 0.0f, 0.0f));
        break;

        case UI_LABEL:
            ui->__cache.glquad = 
                glquad(ui->__cache.quad, 
                        ui->styles.color,
                        quadf(vec3f(0.0f), 0.0f, 0.0f));
        break;

        case UI_DROPDOWN:
        break;

        default: eprint("type not accounted for ");
    }

    ui->__cache.cache_again = false;

}

__ui_cache_t __ui_cache_init(void)
{
    return (__ui_cache_t ) {
        .cache_again    = true,
        .texts          = gltext_init(KB),
    };
}

ui_t __ui_init(const char *label, uitype type, uistyle_t styles)
{
    assert(strlen(label) < 16);
    ui_t o = {
        .title      = {0},
        .type       = type,
        .styles     = styles,
        .is_hot     = false,
        .is_active  = false,    
        .__cache    = __ui_cache_init()
    };
    memcpy(o.title, label, sizeof(o.title));
    return o;
}
