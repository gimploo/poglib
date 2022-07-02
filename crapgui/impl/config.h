#pragma once
#include <poglib/crapgui/decl.h>

void __crapgui_saveall_to_config(crapgui_t *gui)
{
    assert(gui);
    file_t *file = file_init(DEFAULT_CRAPGUI_CONFIG_PATH, "w");
    
    file_writebytes(file, gui, sizeof(crapgui_t ));
    file_writebytes(file, 
            gui->frames.__keys.__array, 
            gui->frames.__keys.__capacity * gui->frames.__keys.__elem_size);
    file_writebytes(file, 
            gui->frames.__values.__values, 
            gui->frames.__values.__capacity * gui->frames.__values.__value_size);

    map_t *frames = &gui->frames;
    map_iterator(frames, iter01)
    {
        frame_t *frame = (frame_t *)iter01;
        file_writebytes(file, frame, sizeof(frame_t ));

        slot_t *uis = &frame->uis;
        slot_iterator(uis, iter02)
        {
            ui_t *ui = (ui_t *)iter02;
            file_writebytes(file, ui, sizeof(ui_t ));
        }
    }
    file_destroy(file);
}

crapgui_t __crapgui_init_from_config(void)
{    
    file_t *file = file_init(DEFAULT_CRAPGUI_CONFIG_PATH, "r");

    crapgui_t tmp;
    file_readbytes(file, &tmp, sizeof(crapgui_t ));
    
    crapgui_t gui = __crapgui_init();
    gui.editmode.is_on = false;
    gui.frames = tmp.frames;
    map_t *frames = &gui.frames;
    map_iterator(frames, iter01)
    {
        frame_t *frame = (frame_t *)iter01;
        file_readbytes(file, frame, sizeof(frame_t ));
        frame->__cache = frame_init(NULL, vec2f(0.0f), (uistyle_t ){0}).__cache;

        frame->uis = slot_init(MAX_UI_CAPACITY_PER_FRAME, ui_t );
        slot_t *uis = &frame->uis;
        slot_iterator(uis, iter02)
        {
            ui_t *ui = (ui_t *)iter02;
            file_readbytes(file, ui, sizeof(ui_t ));
            ui->__cache.texts = gltext_init(KB);
        }
    }
    file_destroy(file);

    return gui;
}


