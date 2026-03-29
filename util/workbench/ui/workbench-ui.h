#pragma once
#include "poglib/application.h"
#include "poglib/basic/color.h"
#include <poglib/gui.h>

void    workbench_render_ui(gui_t *self);


#ifndef IGNORE_WORKBENCH_GUI_RENDER

void workbench_compose_ui(const application_t * const app, gui_t *gui)
{
    const u32 window_width = global_window->width;
    const f32 fps = application_get_fps(app);

    //TODO: rethink on using percentages as measuring unit over strict pixels length
    gui_design_begin(gui); 
            gui_container_begin(gui, str("panel"), UI_TYPE_PANEL, 
                (style_t){
                    .color = COLOR_DARK_GRAY,
                    //ISSUE: keeping same margin for all sides, voids any margin from applying
                    .margin = {10.f, 10.f, 0.f, 10.f },
                    .padding = {5, 5}, 
                    .dim = {
                        .width = window_width,
                        .height = 50
                    },
                .layout = UI_LAYOUT_HORIZONTAL
            });

            //ISSUE: text color doesnt work
            gui_element(
                    gui, 
                    str("FPS: "),
                    UI_TYPE_LABEL, 
                    (style_t){
                        .color = COLOR_WHITE,
                    });

            //ISSUE: Dynamic string doesnt work for label
            gui_element(
                gui, 
                str("329"),
                UI_TYPE_LABEL, 
                (style_t){
                    .color = COLOR_CYAN,
                }
            );

            //ISSUE: issue with multiple panels nested togther, this panel dosent render
            gui_container_begin(gui, str("panel2"), UI_TYPE_PANEL, 
                (style_t){
                    .color = COLOR_RED,
                    .margin = {10.f, 10.f, 0.f, 10.f },
                    .padding = {5, 5}, 
                    .dim = {
                        .width = window_width,
                        .height = 100
                    },
                .layout = UI_LAYOUT_HORIZONTAL
            });
            gui_container_end(gui);
        gui_container_end(gui);
    gui_design_end(gui);
}

#endif
