#pragma once
#include "../decl.h"
#include "./frame.h"
#include "./editmode.h"
#include "./config.h"

#define __crapgui_in_editmode(PGUI)\
    (PGUI)->editmode.is_on

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

bool __crapgui_is_mouse_over_frame(crapgui_t *gui, frame_t *frame)
{
    window_t *win = gui->win;

    vec2f_t norm_mouse_position = window_mouse_get_norm_position(win);
    bool output = quadf_is_point_in_quad(frame->__cache.self.quad, norm_mouse_position);

    frame->__cache.uis.cache_again = output;
    return output;
}

void __crapgui_input_update(crapgui_t *gui)
{
    if (window_mouse_button_just_pressed(gui->win, SDL_MOUSEBUTTON_MIDDLE)) {
            gui->editmode.optionwheel.is_active = 
                !gui->editmode.optionwheel.is_active;
            gui->editmode.optionwheel.pos = 
                window_mouse_get_norm_position(gui->win);
    }

    if (gui->editmode.optionwheel.is_active) return;

    if (window_keyboard_is_key_pressed(gui->win, SDLK_e))
    {
        gui->editmode.focused.ui    = true;
        gui->editmode.focused.frame = false;
    }

    if (window_keyboard_is_key_pressed(gui->win, SDLK_q))
    {
        gui->editmode.focused.frame = true;
        gui->editmode.focused.ui    = false;
    }

    if (window_keyboard_is_key_pressed(gui->win, SDLK_s))
    {
        printf("Saving to config file\n");
        __crapgui_saveall_to_config(gui);
    }

    if (window_keyboard_is_key_pressed(gui->win, SDLK_w))
    {
        gui->editmode.is_on         = !gui->editmode.is_on;
        gui->editmode.focused.frame = true;
        gui->editmode.focused.ui    = false;
        printf("MODE: %s\n", gui->editmode.is_on ? "EDIT" : "NORMAL");
    }

    if (window_mouse_button_is_released(gui->win, SDL_MOUSEBUTTON_LEFT))
    {
        if (gui->editmode.focused.frame)    gui->currently_active.frame = NULL;
        else if (gui->editmode.focused.ui)  gui->currently_active.ui = NULL;
    }

}

void __crapgui_update(crapgui_t *gui)
{
    __crapgui_input_update(gui);
    if (__crapgui_in_editmode(gui)) {
        __crapgui_editmode_update(gui);
        return;
    }

    map_t *map = &gui->frames;
    map_iterator(map, iter) 
    {
        frame_t *frame = (frame_t *)iter;
        if (__crapgui_is_mouse_over_frame(gui, frame))
        {
            gui->currently_active.frame = frame;
            __frame_update(frame, gui);
        }
    }
}

void __crapgui_render(crapgui_t *gui)
{
    if (__crapgui_in_editmode(gui)) {
        __crapgui_editmode_render(gui);
        return;
    }

    map_t *map = &gui->frames;
    map_iterator(map, iter) 
    {
        frame_t *frame = (frame_t *)iter;
        __frame_render(frame,gui);
    }
}

bool __crapgui_check_config_exist(void)
{
    return file_check_exist(DEFAULT_CRAPGUI_CONFIG_PATH);
}

crapgui_t __crapgui_init(void)
{
    return (crapgui_t ) {
        .win                    = global_window,
        .frames                 = NULL,
        .frame_assets = {
            .font               = glfreetypefont_init(DEFAULT_FRAME_FONT_PATH, DEFAULT_FRAME_FONT_SIZE),
            .shader             = glshader_from_file_init(COMMON_VS, FRAME_FS),
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
        .__common_shader        = glshader_from_file_init(COMMON_VS, COMMON_FS),
        .editmode = {
            .is_on              = true, 
            .focused = {
                .frame          = true, 
                .ui             = false, 
            },
        },
        .currently_active = {
            .frame              = NULL,
            .ui                 = NULL
        },
        .update                 = __crapgui_update,
        .render                 = __crapgui_render

    };
}

crapgui_t crapgui_init(void)
{
    if (!global_window) eprint("global window is null");

    if (__crapgui_check_config_exist()) {
        printf("[CRAPGUI] config file found in `%s`\n", 
                DEFAULT_CRAPGUI_CONFIG_PATH);
        return __crapgui_init_from_config();
    } 

    printf("[CRAPGUI] config file not found `%s`\n", DEFAULT_CRAPGUI_CONFIG_PATH);
    crapgui_t output = __crapgui_init();
    output.frames = map_init(MAX_FRAMES_ALLOWED, frame_t );
    return output;
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
    map_t *map      = &gui->frames;
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
    gui->currently_active.ui    = NULL;
    gui->currently_active.frame = NULL;

    glshader_destroy(&gui->__common_shader);
}

