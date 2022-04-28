#pragma once
#include "../decl.h"
#include "./frame.h"

#define COMMON_VS   "lib/poglib/crapgui/res/common.vs"
#define FRAME_FS    "lib/poglib/crapgui/res/frame.fs"
#define BUTTON_FS   "lib/poglib/crapgui/res/button.fs"

void __crapgui_update(crapgui_t *gui)
{
    map_t *map = &gui->frames;
    map_iterator(map, iter) 
    {
        frame_t *frame = (frame_t *)iter;
        frame->update(frame);
    }
}

void __crapgui_render(crapgui_t *gui)
{
    map_t *map = &gui->frames;
    map_iterator(map, iter) 
    {
        frame_t *frame = (frame_t *)iter;
        frame->render(frame);
    }
}

crapgui_t crapgui_init(void)
{
    if (!global_window) eprint("global window is null");

    return (crapgui_t ) {

        .win    = global_window,

        .fonts = {
            [UI_BUTTON] = glfreetypefont_init(DEFAULT_BUTTON_FONT_PATH, DEFAULT_BUTTON_FONT_SIZE),
            [UI_LABEL]  = glfreetypefont_init(DEFAULT_LABEL_FONT_PATH, DEFAULT_LABEL_FONT_SIZE),
            [UI_FRAME]  = glfreetypefont_init(DEFAULT_FRAME_FONT_PATH, DEFAULT_FRAME_FONT_SIZE)
        },

        .shaders = {
            [UI_BUTTON] = glshader_from_file_init(COMMON_VS, BUTTON_FS),
            [UI_LABEL]  = glshader_default_init(),
            [UI_FRAME]  = glshader_from_file_init(COMMON_VS, FRAME_FS),
        },

        .frames = map_init(MAX_FRAMES_ALLOWED, frame_t ),
        .update = __crapgui_update,
        .render = __crapgui_render

    };
}

vec2f_t __crapgui_get_pos_for_new_frame(const crapgui_t *gui)
{
    vec2f_t output      = {0};
    const map_t *frames = &gui->frames;
    vec2f_t margin      = DEFAULT_FRAME_MARGIN;

    if (frames->len == 0) 
        return (vec2f_t ) { -1.0f + margin.cmp[X], 1.0f - margin.cmp[Y] };

    const u64 last_index = frames->len - 1;
    frame_t *prev_frame = (frame_t *)map_get_value_at_index(
                            frames, 
                            last_index);

    if ((prev_frame->pos.cmp[X] + (2.0f * prev_frame->dim.cmp[X]) + margin.cmp[X]) 
            >= 1.0f) {
        output.cmp[X] = -1.0f + margin.cmp[X];
        output.cmp[Y] = 
            prev_frame->pos.cmp[Y] - prev_frame->dim.cmp[Y] - margin.cmp[Y];
    } else {
        output.cmp[X] = 
            prev_frame->pos.cmp[X] + prev_frame->dim.cmp[X] + margin.cmp[X];
        output.cmp[Y] = 
            prev_frame->pos.cmp[Y];
    }

    if (output.cmp[Y] - prev_frame->dim.cmp[Y] - margin.cmp[Y] <= -1.0f)
        eprint("This frame cannot fit in the window");

    return output;

}

frame_t * __crapgui_add_frame(crapgui_t *gui, const char *label)
{
    map_t *map = &gui->frames;

    vec2f_t pos     = __crapgui_get_pos_for_new_frame(gui);
    frame_t frame   = frame_init(
                        gui,
                        label, 
                        pos, 
                        DEFAULT_FRAME_BACKGROUNDCOLOR, 
                        DEFAULT_FRAME_DIMENSIONS);

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

    for (u32 i = 0; i < UITYPE_COUNT; i++)
    {
        glshader_t *shader      = &gui->shaders[i];
        glfreetypefont_t *font  = &gui->fonts[i];

        glshader_destroy(shader);
        glfreetypefont_destroy(font);
    }
}
