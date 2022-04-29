#pragma once
#include "./crapgui/impl/manager.h"
#include "./crapgui/impl/button.h"
#include "./crapgui/impl/frame.h"

//TODO: fix relative frame mouse position
//TODO: desgin better state managamenet (__is_change ) shit
//TODO: frame design 

crapgui_t   crapgui_init(void);
#define     crapgui_update(PGUI)                (PGUI)->update(PGUI)
#define     crapgui_render(PGUI)                (PGUI)->render(PGUI)
void        crapgui_destroy(crapgui_t *gui);

#define crapgui_layout(PGUI)\
        for (crapgui_t *__gui = (PGUI); __gui != NULL; __gui = NULL)

#define frame(LABEL)\
    assert(__gui);\
    for (frame_t *__frame = __crapgui_add_frame(__gui, LABEL); __frame != NULL; __frame = NULL)

#define button(LABEL)\
    assert(__frame);\
    __crapgui_frame_add_uielem(__gui, __frame, LABEL, UI_BUTTON)

#define label(LABEL)\
    assert(__frame);\
    __crapgui_frame_add_uielem(__gui, __frame, LABEL, UI_LABEL)

