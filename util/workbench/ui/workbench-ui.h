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


    ui_compose_begin(gui, (ui_config_t){ 
        .composition = {
            .styles = UI_STYLE_ROUNDED_CORNERS,
            .traits = UI_BEHAVIOR_HOVERABLE | UI_BEHAVIOR_CLICKABLE
        },
        .color = {
            .base = COLOR_BLUE,
            .highlight = COLOR_RED,
        },
        .dim = {
            .height = 20,
            .width = 20
        },
        .padding = {
            .left = 1,
            .bottom = 1,
            .right = 1,
            .top = 1
        },
        .margin = {
            .left = 5, 
            .right = 5,
            .top = 5,
            .bottom = 5
        }
    });
    /*
        ui_compose_begin(gui, (ui_config_t) {
            .composition = {0},
            .color = {
                .base = COLOR_WHITE, 
                .highlight = COLOR_BLACK
            },
            .label = str("Hello world")
        });
        ui_compose_end(gui);
        */
    ui_compose_end(gui);
}

#endif
