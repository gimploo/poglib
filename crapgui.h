#pragma once
#include "./crapgui/impl/manager.h"
#include "./crapgui/impl/frame.h"

//FIXME: the lable is alone in a frame, it doesnt render the color behind the text 
//TODO: when checking whether the mouse is over the button, have a system that only checks for those in that region and not the entire frame
//TODO: MAKE A BATCH MANAGER URGENT !!!!!!!!!!!!!!!!!!!!!!
//TODO: move all edit mode stuff into a separate compartment

crapgui_t   crapgui_init(void);
#define     crapgui_update(PGUI)                (PGUI)->update(PGUI)
#define     crapgui_render(PGUI)                (PGUI)->render(PGUI)
void        crapgui_destroy(crapgui_t *gui);

#define crapgui_layout(PGUI)\
        for (crapgui_t *__gui = (PGUI); __gui != NULL; __gui = NULL)

#define frame(LABEL, STYLES)\
    assert(__gui);\
    for (frame_t *__frame = __crapgui_add_frame(__gui, LABEL, (STYLES)); __frame != NULL; __frame_update(__frame, __gui), __frame = NULL)


#define button(LABEL, STYLES)\
    assert(__frame);\
    assert(__gui);\
    __frame_add_ui(__frame, __gui, LABEL, UI_BUTTON, (STYLES))

#define label(LABEL, STYLES)\
    assert(__frame);\
    assert(__gui);\
    __frame_add_ui(__frame, __gui, LABEL, UI_LABEL, (STYLES))


#define crapgui_get_button(PGUI, FLABEL, BLABEL)                              __impl_crapgui_get_button_from_frame((PGUI), (FLABEL), (BLABEL)) 
