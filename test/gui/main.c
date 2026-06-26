#include <poglib/gui.h>

int main(void)
{
    const application_t * const app = NULL;
    gui_t *gui = NULL;

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
            .min_height = 100,
            .min_width = 100 
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
            .composition = {
                .styles = UI_STYLE_ONLY_TEXT
            },
            .color = {
                .base = COLOR_WHITE,
                .highlight = COLOR_CYAN,
            },
            .dim = {
                .min_height = 40,
                .min_width = 80 
            },
            .padding = {0},
            .margin = {0},
            .text = str("Hello World")
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
                .min_height = 50,
                .min_width = 50
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
                    .min_height = 10,
                    .min_width = 10
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
