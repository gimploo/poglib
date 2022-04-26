#pragma once
#include "../decl.h"


void __button_update(button_t *button, uielem_t *uielem)
{
    assert(button);

    uielem->__vertices = 
        glquad(quadf(vec3f(uielem->pos), uielem->dim.cmp[X], uielem->dim.cmp[Y]),
                button->is_hot ? button->hover_color : uielem->color,
                quadf(vec3f(0.0f), 0.0f, 0.0f),
                0);
}


button_t button_init(const char *label) 
{
    button_t button = {
        .label          = label,
        .is_active      = false,
        .is_hot         = false,
        .hover_color    = DEFAULT_BUTTON_HOVER_COLOR,
        .update         = __button_update,
    };

    return button;
}
