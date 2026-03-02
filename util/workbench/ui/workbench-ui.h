#pragma once
#include <poglib/gui.h>

void    workbench_render_ui(gui_t *self);


#ifndef IGNORE_WORKBENCH_GUI_RENDER

void workbench_compose_ui(gui_t *gui)
{
    gui_design_begin(gui); 
        gui_container_begin(gui, str("panel"), UI_TYPE_PANEL, 
            (style_t){
                .color = COLOR_GRAY,
                .margin = {10.f, 10.f, 0.f, 0.f},
                .padding = {10, 10}, 
                .dim = {
                .width = 200,
                .height = 400
            },
            .layout = UI_LAYOUT_VERTICAL
        });
            gui_element(
                gui, 
                str("button1"), 
                UI_TYPE_BUTTON, 
                (style_t){0});

            gui_element(
                gui, 
                str("button2"), 
                UI_TYPE_BUTTON, 
                (style_t){0});
            gui_element(
                gui, 
                str("button3"), 
                UI_TYPE_BUTTON, 
                (style_t){0});

            gui_element(
                gui, 
                str("button4"), 
                UI_TYPE_BUTTON, 
                (style_t){0});

        gui_container_end(gui);
    gui_design_end(gui);
}

#endif
