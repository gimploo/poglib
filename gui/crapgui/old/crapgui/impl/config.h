#pragma once
#include <poglib/crapgui/decl.h>
#include <stddef.h>
#include "frame.h"

void __crapgui_saveall_to_config(crapgui_t *gui)
{
    assert(gui);
    file_t file = file_init(DEFAULT_CRAPGUI_CONFIG_PATH, "wb");

    file_save_map(&file, gui->frames);
    map_iterator(&gui->frames, iter01)
    {
        frame_t *frame = (frame_t *)iter01;
        file_save_slot(&file, frame->uis);
    }
    file_destroy(&file);
}

crapgui_t __crapgui_init_from_config(void)
{    
    file_t file = file_init(DEFAULT_CRAPGUI_CONFIG_PATH, "rb");

    crapgui_t gui       = __crapgui_init();
    gui.frames          = file_load_map(&file);
    gui.editmode.is_on  = false;

    map_iterator(&gui.frames, iter01)
    {
        frame_t *frame  = (frame_t *)iter01;
        frame->uis      = file_load_slot(&file);
        frame->__cache  = __frame_cache_init();

        slot_iterator(&frame->uis, iter02)
        {
            ui_t *ui = (ui_t *)iter02;
            ui->__cache = __ui_cache_init();
        }

        __crapgui_recache_both(frame);

        __frame_update(frame, &gui);
    }
    file_destroy(&file);

    return gui;
}


