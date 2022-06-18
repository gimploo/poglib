#pragma once
#include "../decl.h"

#define __crapgui_in_editmode(PGUI)\
    (PGUI)->editmode.is_on

#define __crapgui_editmode_set_active_frame(PGUI, FRAME)\
    (PGUI)->editmode.active_frame = (FRAME)

#define __crapgui_editmode_is_keypressed(PGUI, KEY) do {\
    if (window_keyboard_is_key_just_pressed((PGUI)->win, (KEY)))\
        (PGUI)->editmode.is_on = !(PGUI)->editmode.is_on;\
} while (0)


void __crapgui_editmode_update(crapgui_t *gui)
{
    if (!gui->editmode.active_frame) eprint("active frame is null");

    if (window_mouse_button_is_released(gui->win))
        gui->editmode.active_ui = NULL;

    frame_t *active_frame = gui->editmode.active_frame;
    active_frame->update(active_frame, gui);
}

void __crapgui_editmode_render(crapgui_t *gui)
{
    if (!gui->editmode.active_frame) eprint("active frame is null");

    const frame_t *frame = gui->editmode.active_frame;

    vec3f_t pos = {-1.0f, 1.0f, 0.0f};
    glquad_t quad = glquad(
                quadf(pos, 2.0f, 2.0f),
                frame->styles.color,
                quadf(vec3f(0.0f), 1.0f, 1.0f), 
                0);
    glrenderer2d_t r2d = {
        .shader = &gui->frame_assets.shader,
        .texture = &gui
                    ->editmode.active_frame
                    ->__frame_ui_cache.texture.texture2d,
    };

    glrenderer2d_draw_quad(&r2d, quad);
}

void __crapgui_editmode_ui_is_mouse_over(ui_t *ui, const frame_t *frame, crapgui_t *gui)
{
    if (gui->editmode.active_ui) return;

    const vec2f_t mousepos  = window_mouse_get_norm_position(global_window);
    const quadf_t rect      = quadf(
                                vec3f(ui->pos), 
                                ui->styles.width, 
                                ui->styles.height); 
    window_t *win = gui->win;

    if(window_mouse_button_is_held(win) 
        && quadf_is_point_in_quad(rect, mousepos))
    {
        gui->editmode.active_ui = ui;

    } 
}

void __crapgui_editmode_ui_is_mouse_held(const frame_t *frame, const crapgui_t *gui)
{
    const vec2f_t mousepos  = window_mouse_get_norm_position(global_window);
    const window_t *win     = window_get_current_active_window();

    if (gui->editmode.active_ui 
        && window_mouse_button_is_held(global_window))
    {
        ui_t *ui = gui->editmode.active_ui;
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
