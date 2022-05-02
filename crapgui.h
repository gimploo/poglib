#pragma once
#include "./crapgui/impl/manager.h"
#include "./crapgui/impl/frame.h"

//FIXME: the lable is alone in a frame, it doesnt render the color behind the text 
//TODO: when checking whether the mouse is over the button, have a system that only checks for those in that region and not the entire frame
//TODO: ui/ux frame design 
//TODO: have glfreetypefont accept opengl coordinates for position

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
    __frame_add_ui(__frame, LABEL, UI_BUTTON)

#define label(LABEL)\
    assert(__frame);\
    __frame_add_ui(__frame, LABEL, UI_LABEL)


#define crapgui_get_button(PGUI, FLABEL, BLABEL)                              __impl_crapgui_get_button_from_frame((PGUI), (FLABEL), (BLABEL)) 
