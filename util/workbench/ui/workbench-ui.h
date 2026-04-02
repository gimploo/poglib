#pragma once
#include "poglib/application.h"
#include "poglib/application/window/sdl_window.h"
#include "poglib/basic/color.h"
#include "poglib/basic/str.h"
#include <poglib/gui.h>
#include <stdio.h>
#include <string.h>

void    workbench_render_ui(gui_t *self);


#ifndef IGNORE_WORKBENCH_GUI_RENDER

void workbench_compose_ui(const application_t * const app, gui_t *gui, vec3f_t camera_pos)
{
    const u32 window_width = global_window->width;
    char tempbuffer[32] = {0};

    //FPS Counter
    gui_ui_compose_begin(gui, (ui_config_t){ 
        .composition = {
            .styles = UI_STYLE_ROUNDED_CORNERS,
        },
        .color = {
            .base = COLOR_BLACK,
        },
        .dim = {
            .height = 30,
            .width = 80 
        },
        .padding = {4,4,4,4},
        .margin = {
            .left = 10, 
            .right = 10,
            .top = 10,
            .bottom = 10 
        }
    }); 
    {
        const f32 fps = application_get_fps(app);
        snprintf(tempbuffer, sizeof(tempbuffer), "FPS: %d", (int)fps);
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {0},
            .color = {
                .base = COLOR_GRAY,
            },
            .dim = {
                .height = 40,
                .width = 40 
            },
            .padding = {0},
            .margin = {0},
            .label = str__from_cstr(tempbuffer, sizeof(tempbuffer))
        });
        gui_ui_compose_end(gui);
    } 
    gui_ui_compose_end(gui);

    //Mouse position normalized
    gui_ui_compose_begin(gui, (ui_config_t){ 
        .composition = {
            .styles = UI_STYLE_ROUNDED_CORNERS,
        },
        .color = {
            .base = COLOR_BLACK,
        },
        .dim = {
            .height = 30,
            .width = 140 
        },
        .padding = {8,4,4,4},
        .margin = {
            .left = 5, 
            .right = 5,
            .top = 10,
            .bottom = 10 
        }
    }); 
    {
        const vec2f_t mouse_pos = window_mouse_get_norm_position(global_window);
        memset(tempbuffer, 0, sizeof(tempbuffer));
        snprintf(tempbuffer, sizeof(tempbuffer), "NDC [ %.2f, %.2f ]", mouse_pos.x, mouse_pos.y);
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {0},
            .color = {
                .base = COLOR_GRAY,
            },
            .dim = {
                .height = 20,
                .width = 100 
            },
            .padding = {0},
            .margin = {2,2,2,2},
            .label = str__from_cstr(tempbuffer, sizeof(tempbuffer))
        });
        gui_ui_compose_end(gui);

        memset(tempbuffer, 0, sizeof(tempbuffer));
        const vec2i_t mouse_pos_wc = window_mouse_get_position(global_window);
        snprintf(tempbuffer, sizeof(tempbuffer), "WDC [ %i, %i ]", mouse_pos_wc.x, mouse_pos_wc.y);
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {0},
            .color = {
                .base = COLOR_GRAY,
            },
            .dim = {
                .height = 20,
                .width = 100 
            },
            .padding = {0},
            .margin = {0},
            .label = str__from_cstr(tempbuffer, sizeof(tempbuffer))
        });
        gui_ui_compose_end(gui);
    } 
    gui_ui_compose_end(gui);

    //Camera pos
    gui_ui_compose_begin(gui, (ui_config_t){ 
        .composition = {
            .styles = UI_STYLE_ROUNDED_CORNERS,
        },
        .color = {
            .base = COLOR_BLACK,
        },
        .dim = {
            .height = 30,
            .width = 200 
        },
        .padding = {8,4,4,4},
        .margin = {
            .left = 5, 
            .right = 5,
            .top = 10,
            .bottom = 10 
        }
    }); 
    {
        const vec2f_t mouse_pos = window_mouse_get_norm_position(global_window);
        memset(tempbuffer, 0, sizeof(tempbuffer));
        snprintf(tempbuffer, sizeof(tempbuffer), "cam pos [ %.2f, %.2f, %.2f ]", camera_pos.x, camera_pos.y, camera_pos.z);
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {0},
            .color = {
                .base = COLOR_GRAY,
            },
            .dim = {
                .height = 40,
                .width = 80 
            },
            .padding = {0},
            .margin = {0},
            .label = str__from_cstr(tempbuffer, sizeof(tempbuffer))
        });
        gui_ui_compose_end(gui);
    } 
    gui_ui_compose_end(gui);
}

void workbench_compose_ui__test(const application_t * const app, gui_t *gui)
{
    const u32 window_width = global_window->width;
    const f32 fps = application_get_fps(app);

    gui_ui_compose_begin(gui, (ui_config_t){ 
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
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {0},
            .color = {
                .base = COLOR_WHITE,
                .highlight = COLOR_CYAN,
            },
            .dim = {
                .height = 40,
                .width = 80 
            },
            .padding = {0},
            .margin = {0},
            .label = str("Hello World")
        });
        gui_ui_compose_end(gui);
    gui_ui_compose_end(gui);

    for(u8 i = 0; i < 100; i++) {
        gui_ui_compose_begin(gui, (ui_config_t){ 
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
            gui_ui_compose_begin(gui, (ui_config_t){ 
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
            gui_ui_compose_end(gui);
        gui_ui_compose_end(gui);
    }
}

#endif
