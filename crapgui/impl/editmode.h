#pragma once
#include "../decl.h"
#include "editmode/editwheel.h"


void __crapgui_editmode_render(crapgui_t *gui)
{
    frame_t *frame = gui->currently_active.frame;

    if (gui->editmode.focused.frame) {

        map_t *map = &gui->frames;
        map_iterator(map, iter) 
        {
            frame_t *frame = (frame_t *)iter;
            frame->render(frame,gui);
        }
        
    } else if (gui->editmode.focused.ui) {

        assert(frame);
        vec3f_t pos = {-1.0f, 1.0f, 0.0f};
        glquad_t quad = glquad(
                    quadf(pos, 2.0f, 2.0f),
                    frame->styles.color,
                    quadf(vec3f(0.0f), 1.0f, 1.0f), 
                    0);
        glrenderer2d_t r2d = {
            .shader = &gui->frame_assets.shader,
            .texture = &frame->__cache.uis.texture.texture2d,
        };

        glrenderer2d_draw_quad(&r2d, quad);

    } 

    /*editwheel_render();*/
}

void __crapgui_editmode_frame_is_mouse_over(crapgui_t *gui, frame_t *frame)
{
    vec2f_t norm_mouse_position = window_mouse_get_norm_position(gui->win);

    if (quadf_is_point_in_quad(frame->__cache.self.quad, norm_mouse_position))
        gui->currently_active.frame = frame;
}

bool __crapgui_editmode_ui_is_mouse_over(ui_t *ui, crapgui_t *gui)
{
    const vec2f_t mousepos  = window_mouse_get_norm_position(gui->win);
    const quadf_t rect      = quadf(
                                vec3f(ui->pos), 
                                ui->styles.width, 
                                ui->styles.height); 

    if(quadf_is_point_in_quad(rect, mousepos) 
        && window_mouse_button_just_pressed(gui->win, SDL_MOUSEBUTTON_LEFT))
    {
        gui->currently_active.ui = ui;
        return true;
    }
    return false;
}

void __crapgui_editmode_frame_is_mouse_held(const crapgui_t *gui)
{
    frame_t *frame = gui->currently_active.frame;
    if (!frame) return;

    const vec2f_t mousepos  = window_mouse_get_norm_position(gui->win);
    if (window_mouse_button_is_held(gui->win, SDL_MOUSEBUTTON_LEFT))
    {
        const vec2f_t dim = { 
            frame->styles.width / 2.0f, 
            frame->styles.height / 2.0f 
        };

        frame->pos = (vec2f_t ){
            mousepos.cmp[X] - dim.cmp[X],
            mousepos.cmp[Y] + dim.cmp[Y],
        };
        __crapgui_recache_only_frame(frame);
    } 
}

void __crapgui_editmode_ui_is_mouse_held(const crapgui_t *gui)
{
    frame_t *frame  = gui->currently_active.frame;
    ui_t *ui        = gui->currently_active.ui;
    if (!ui) return;

    const vec2f_t mousepos  = window_mouse_get_norm_position(gui->win);
    if (window_mouse_button_is_held(gui->win, SDL_MOUSEBUTTON_LEFT))
    {
        const vec2f_t dim = { 
            ui->styles.width / 2.0f, 
            ui->styles.height / 2.0f 
        };

        ui->pos = (vec2f_t ){
            mousepos.cmp[X] - dim.cmp[X],
            mousepos.cmp[Y] + dim.cmp[Y],
        };
        __crapgui_recache_ui(ui);
        __crapgui_recache_only_uis(frame);
    }
}



void __crapgui_editmode_update(crapgui_t *gui)
{
    frame_t *active_frame = gui->currently_active.frame;
    ui_t    *active_ui    = gui->currently_active.ui;

    if (gui->editmode.focused.ui) 
    {
        assert(active_frame);
        const slot_t *uis = &active_frame->uis;
        slot_iterator(uis, iter)
        {
            ui_t *tmp = (ui_t *)iter;
            if (__crapgui_editmode_ui_is_mouse_over(tmp, gui))
                break;
        }
        __crapgui_editmode_ui_is_mouse_held(gui);

    } else if (gui->editmode.focused.frame) {

        const map_t *map = &gui->frames;
        map_iterator(map, iter) 
        {
            frame_t *tmp = (frame_t *)iter;
            __crapgui_editmode_frame_is_mouse_over(gui, tmp);
        }
        __crapgui_editmode_frame_is_mouse_held(gui);
        if (!active_frame) return;
    } 

    active_frame->update(active_frame, gui);

}
