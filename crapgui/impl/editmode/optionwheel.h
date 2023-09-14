#pragma once
#include <poglib/application.h>
#include <poglib/math.h>
#include "../../decl.h"

#define EDIT_WHEEL_RADIUS 0.2f

typedef enum optiontype {

    OT_FRAME_EDIT,
    OT_UI_EDIT,
    OT_EDIT_OPTION_01,
    OT_EDIT_OPTION_02,
    OT_EDIT_OPTION_03,
    OT_EDIT_OPTION_001,
    OT_EDIT_OPTION_002,
    OT_EDIT_OPTION_003,
    OT_EDIT_OPTION_004,
    OT_EDIT_OPTION_0003,
    OT_EDIT_OPTION_0004,
    OT_COUNT,

} optiontype;


void __crapgui_editmode_editwheel_update(crapgui_t *gui)
{
}

void __crapgui_editmode_editwheel_render(const crapgui_t *gui)
{
    if (!gui->editmode.optionwheel.is_active) return;

    vec2f_t mpos    = gui->editmode.optionwheel.pos;
    polygon_t wheel = polygon((vec3f_t ){ mpos.x, mpos.y, 0.f }, EDIT_WHEEL_RADIUS, OT_COUNT);


    glrenderer2d_draw_polygon(
                & (glrenderer2d_t ) {
                    .shader = &gui->__common_shader,
                    .texture = NULL 
                }, 
                glpolygon(wheel, COLOR_BLUE, quadf(vec3f(0.0f), 0.0f, 0.0f))
            );
}



