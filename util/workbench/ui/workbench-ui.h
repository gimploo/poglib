#pragma once
#include "poglib/application.h"
#include "poglib/application/window/sdl_window.h"
#include "poglib/basic/color.h"
#include "poglib/basic/str.h"
#include "poglib/poggen.h"
#include <poglib/gui.h>


void workbench_editor_render_header(gui_t *const gui, const vec3f_t camera_pos, const vec2f_t cam_orientation, bool *const enable_collider)
{
    char tempbuffer[1024] = {0};

    gui_ui_compose_begin(gui, (ui_config_t){
        .color = {
            .base = COLOR_WHITE,
        },
        .dim = {
            .min_height = 50,
            .min_width = global_window->width
        },
    });
    //FPS Counter
    gui_ui_compose_begin(gui, (ui_config_t){ 
        .composition = {
            .styles = UI_STYLE_ROUNDED_CORNERS,
        },
        .color = {
            .base = COLOR_BLACK,
        },
        .dim = {
            .min_height = 30,
            .min_width = 80 
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
        const f32 fps = application_get_fps(global_engine->handle.app);
        snprintf(tempbuffer, sizeof(tempbuffer), "FPS: %d", (int)fps);
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {
                .styles = UI_STYLE_ONLY_TEXT
            },
            .color = {
                .base = COLOR_OFFWHITE,
            },
            .dim = {
                .min_height = 40,
                .min_width = 40 
            },
            .padding = {0},
            .margin = {0},
            .text = str_from_cstr(tempbuffer, sizeof(tempbuffer))
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
            .min_height = 30,
            .min_width = 250
        },
        .padding = {4,4,4,8},
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
        snprintf(tempbuffer, sizeof(tempbuffer), "NDC [ %.2f, %.2f ] | ", mouse_pos.x, mouse_pos.y);
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {
                .styles = UI_STYLE_ONLY_TEXT
            },
            .color = {
                .base = COLOR_SOFT_WHITE,
            },
            .dim = {
                .min_height = 20,
                .min_width = 120 
            },
            .padding = {0},
            .margin = {2,2,2,2},
            .text = str_from_cstr(tempbuffer, sizeof(tempbuffer))
        });
        gui_ui_compose_end(gui);

        memset(tempbuffer, 0, sizeof(tempbuffer));
        const vec2i_t mouse_pos_wc = window_mouse_get_position(global_window);
        snprintf(tempbuffer, sizeof(tempbuffer), "WDC [ %i, %i ]", mouse_pos_wc.x, mouse_pos_wc.y);
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {
                .styles = UI_STYLE_ONLY_TEXT
            },
            .color = {
                .base = COLOR_SOFT_WHITE,
            },
            .dim = {
                .min_height = 20,
                .min_width = 100 
            },
            .padding = {0},
            .margin = {2,2,2,2},
            .text = str_from_cstr(tempbuffer, sizeof(tempbuffer))
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
            .min_height = 30,
            .min_width = 200 
        },
        .padding = {4,4,4,8},
        .margin = {
            .left = 5, 
            .right = 5,
            .top = 10,
            .bottom = 10 
        }
    }); 
    {
        memset(tempbuffer, 0, sizeof(tempbuffer));
        snprintf(tempbuffer, sizeof(tempbuffer), "cam pos [ %.2f, %.2f, %.2f ]", camera_pos.x, camera_pos.y, camera_pos.z);
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {
                .styles = UI_STYLE_ONLY_TEXT
            },
            .color = {
                .base = COLOR_SOFT_WHITE,
            },
            .dim = {
                .min_height = 40,
                .min_width = 80 
            },
            .padding = {0},
            .margin = {0},
            .text = str_from_cstr(tempbuffer, sizeof(tempbuffer))
        });
        gui_ui_compose_end(gui);
    } 
    gui_ui_compose_end(gui);

    //NOTE: Camera euler angle 
    gui_ui_compose_begin(gui, (ui_config_t){ 
        .composition = {
            .styles = UI_STYLE_ROUNDED_CORNERS,
        },
        .color = {
            .base = COLOR_BLACK,
        },
        .dim = {
            .min_height = 30,
            .min_width = 210 
        },
        .padding = {4,4,4,8},
        .margin = {
            .left = 5, 
            .right = 5,
            .top = 10,
            .bottom = 10 
        }
    }); 
    {
        memset(tempbuffer, 0, sizeof(tempbuffer));
        snprintf(tempbuffer, sizeof(tempbuffer), "cam euler angle [ %.2f, %.2f ]", cam_orientation.x, cam_orientation.y);
        gui_ui_compose_begin(gui, (ui_config_t){ 
            .composition = {
                .styles = UI_STYLE_ONLY_TEXT
            },
            .color = {
                .base = COLOR_SOFT_WHITE,
            },
            .dim = {
                .min_height = 40,
                .min_width = 80 
            },
            .padding = {0},
            .margin = {0},
            .text = str_from_cstr(tempbuffer, sizeof(tempbuffer))
        });
        gui_ui_compose_end(gui);
    } 
    gui_ui_compose_end(gui);

    gui_ui_compose_begin(gui, (ui_config_t) {
        .composition = {
            .styles = UI_STYLE_ROUNDED_CORNERS,
            .traits = UI_BEHAVIOR_CLICKABLE | UI_BEHAVIOR_HOVERABLE | UI_BEHAVIOR_TRACK_STATE_TOGGLE
        },
        .binding = {
            .ref = enable_collider,
            .size = sizeof(bool)
        },
        .dim = {
            .min_height = 30,
            .min_width = 120,
        },
        .color = {
            .base = COLOR_WHITE,
            .highlight = COLOR_GRAY
        },
        .margin = {
            .left = 5, 
            .right = 5,
            .top = 10,
            .bottom = 10 
        },
        .padding = {
            .left = 5,
            .top = 5,
            .bottom = 5,
            .right = 5
        }
    });{
        gui_ui_compose_begin(gui, (ui_config_t) {
            .composition = {
                .styles = UI_STYLE_ONLY_TEXT
            },
            .dim = {
                .min_height = 40,
                .min_width = 40,
            },
            .color = {
                .base = COLOR_BLACK,
            },
            .text = str("show colliders"),
        });
        gui_ui_compose_end(gui);
    }
    gui_ui_compose_end(gui);
    gui_ui_compose_end(gui);
}

