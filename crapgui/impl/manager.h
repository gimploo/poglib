#pragma once
#include "../decl.h"
#include "./frame.h"


#define COMMON_VS   "lib/poglib/crapgui/res/common.vs"
#define COMMON_FS   "lib/poglib/crapgui/res/common.fs"
#define FRAME_FS    "lib/poglib/crapgui/res/frame.fs"
#define BUTTON_FS   "lib/poglib/crapgui/res/button.fs"
#define LABEL_FS   "lib/poglib/crapgui/res/label.fs"

ui_t * __impl_crapgui_get_button_from_frame(const crapgui_t *gui, const char *frame_label, const char *button_label)
{
    assert(frame_label);
    assert(button_label);

    const map_t *map = &gui->frames;
    frame_t *frame = (frame_t *)map_get_value(map, frame_label);

    const slot_t *slot = &frame->uis;
    slot_iterator(slot, iter) 
    {
        ui_t *ui = (ui_t *)iter;
        if (ui->type != UI_BUTTON) continue;
        
        if (strcmp(ui->title, button_label) == 0) 
            return ui;
    }
    eprint("no button (%s) found in frame (%s)", button_label, frame->label);
}

bool __crapgui_is_mouse_over_frame(const crapgui_t *gui, const frame_t *frame)
{
    window_t *win = gui->win;

    vec2f_t norm_mouse_position = window_mouse_get_norm_position(win);
    return quadf_is_point_in_quad(frame->__frame_cache.quad, norm_mouse_position);
}


void __crapgui_update(crapgui_t *gui)
{
    if (window_keyboard_is_key_just_pressed(global_window, SDLK_e))
        gui->edit_mode.is_on = !gui->edit_mode.is_on;

    map_t *map = &gui->frames;
    map_iterator(map, iter) 
    {
        frame_t *frame = (frame_t *)iter;

        if (__crapgui_is_mouse_over_frame(gui, frame))
        {
            if (gui->edit_mode.is_on && gui->edit_mode.active_frame) {
                frame->update(gui->edit_mode.active_frame, gui);
            } else {
                gui->edit_mode.active_frame = frame;
                frame->update(frame, gui);
            }
        }
    }
}

void __crapgui_render(crapgui_t *gui)
{
    map_t *map = &gui->frames;
    map_iterator(map, iter) 
    {
        frame_t *frame = (frame_t *)iter;

        if(gui->edit_mode.is_on && gui->edit_mode.active_frame) 
            frame->render(gui->edit_mode.active_frame, gui);
        else
            frame->render(frame,gui);
    }
}

crapgui_t crapgui_init(void)
{
    if (!global_window) eprint("global window is null");

    return (crapgui_t ) {

        .win                = global_window,
        .frames             = map_init(MAX_FRAMES_ALLOWED, frame_t ),
        .frame_assets = {
            .font           = glfreetypefont_init(DEFAULT_FRAME_FONT_PATH, DEFAULT_FRAME_FONT_SIZE),
            .shader         = glshader_from_file_init(COMMON_VS, FRAME_FS),
        },
        .ui_assets = {
            .fonts = {
                [UI_BUTTON]     = glfreetypefont_init(DEFAULT_BUTTON_FONT_PATH, DEFAULT_BUTTON_FONT_SIZE),
                [UI_LABEL]      = glfreetypefont_init(DEFAULT_LABEL_FONT_PATH, DEFAULT_LABEL_FONT_SIZE),
            },
            .shaders = {
                [UI_BUTTON]     = glshader_from_file_init(COMMON_VS, BUTTON_FS),
                [UI_LABEL]      = glshader_from_file_init(COMMON_VS, LABEL_FS),
            },
        },
        .__common_shader    = glshader_from_file_init(COMMON_VS, COMMON_FS),
        .edit_mode = {
            .is_on          = true, 
            .active_frame   = NULL,
        },
        .update             = __crapgui_update,
        .render             = __crapgui_render

    };
}


vec2f_t __crapgui_get_pos_for_new_frame(const crapgui_t *gui)
{
    vec2f_t output       = {0};
    const map_t *frames  = &gui->frames;

    const vec2f_t df_margin = DEFAULT_FRAME_MARGIN;
    if (frames->len == 0) 
        return (vec2f_t ) { -1.0f + df_margin.cmp[X], 1.0f - df_margin.cmp[Y] };

    const u64 last_index = frames->len - 1;
    frame_t *prev_frame = (frame_t *)map_get_value_at_index(
                            frames, 
                            last_index);

    if ((prev_frame->pos.cmp[X] + (2.0f * prev_frame->styles.width) + prev_frame->styles.margin.cmp[X]) 
            >= 1.0f) {
        output.cmp[X] = -1.0f + prev_frame->styles.margin.cmp[X];
        output.cmp[Y] = 
            prev_frame->pos.cmp[Y] - prev_frame->styles.height - prev_frame->styles.margin.cmp[Y];
    } else {
        output.cmp[X] = 
            prev_frame->pos.cmp[X] + prev_frame->styles.width + prev_frame->styles.margin.cmp[X];
        output.cmp[Y] = 
            prev_frame->pos.cmp[Y];
    }

    if (output.cmp[Y] - prev_frame->styles.height - prev_frame->styles.margin.cmp[Y] <= -1.0f)
        eprint("This frame cannot fit in the window");

    return output;

}

frame_t * __crapgui_add_frame(crapgui_t *gui, const char *label, uistyle_t style)
{
    map_t *map = &gui->frames;

    vec2f_t pos     = __crapgui_get_pos_for_new_frame(gui);
    frame_t frame   = frame_init(
                        label, 
                        pos, style);

    __frame_update(&frame, gui);

    return (frame_t *)map_insert(map, label, frame);
}

void crapgui_destroy(crapgui_t *gui)
{
    assert(gui);

    gui->win = NULL;

    map_t *map = &gui->frames;
    map_iterator(map, iter) {
        frame_t *frame = (frame_t *)iter;
        frame_destroy(frame);
    }
    map_destroy(map);

    glfreetypefont_destroy(&gui->frame_assets.font);
    glshader_destroy(&gui->frame_assets.shader);

    for (u32 i = 0; i < UITYPE_COUNT; i++)
    {
        glshader_t *shader      = &gui->ui_assets.shaders[i];
        glfreetypefont_t *font  = &gui->ui_assets.fonts[i];

        glshader_destroy(shader);
        glfreetypefont_destroy(font);
    }

    glshader_destroy(&gui->__common_shader);
}

