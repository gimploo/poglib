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
                .height = 100,
                .width = 100 
            },
            .padding = {
                10, 10, 10, 10
            },
            .margin = {
                .left = 10, 
                .right = 10,
                .top = 10,
                .bottom = 10 
            }
        });
            ui_compose_begin(gui, (ui_config_t){ 
                .composition = {0},
                .color = {
                    .base = COLOR_WHITE,
                    .highlight = COLOR_WHITE,
                },
                .dim = {
                    .height = 40,
                    .width = 80 
                },
                .padding = {0},
                .margin = {0}
            });
            ui_compose_end(gui);
        ui_compose_end(gui);

    for(u8 i = 0; i < 100; i++) {
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
                .height = 50,
                .width = 50
            },
            .padding = {0},
            .margin = {
                .left = 10, 
                .right = 10,
                .top = 10,
                .bottom = 10 
            }
        });
            ui_compose_begin(gui, (ui_config_t){ 
                .composition = {
                    .traits = UI_BEHAVIOR_HOVERABLE
                },
                .color = {
                    .base = COLOR_WHITE,
                    .highlight = COLOR_BLACK,
                },
                .dim = {
                    .height = 10,
                    .width = 10
                },
                .padding = {0},
                .margin = {
                    .left = 10, 
                    .right = 10,
                    .top = 10,
                    .bottom = 10 
                }
            });
            ui_compose_end(gui);
        ui_compose_end(gui);
    }
}

#endif
