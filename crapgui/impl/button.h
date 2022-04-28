#pragma once
#include "../decl.h"


void __button_update(uielem_t *uielem)
{
    assert(uielem);
    assert(uielem->type == UI_BUTTON);

    button_t *button = (button_t *)uielem->value;

    uielem->__uivertices = 
        glquad(quadf(vec3f(uielem->pos), uielem->dim.cmp[X], uielem->dim.cmp[Y]),
                button->is_hot ? button->hover_color : uielem->color,
                quadf(vec3f(0.0f), 0.0f, 0.0f),
                0);

    vec2f_t centerpos = {
        .cmp[X] = uielem->pos.cmp[X] + uielem->dim.cmp[X] / 2,
        .cmp[Y] = uielem->pos.cmp[Y] - uielem->dim.cmp[Y]
    };


    glfreetypefont_t *font = uielem->__font;
    glbatch_t *txtbatch = &uielem->__textbatch;
    glbatch_clear(txtbatch);
    glfreetypefont_add_text_to_batch(
            font, txtbatch, button->label, centerpos, COLOR_WHITE);
}


button_t button_init(const char *label) 
{
    button_t button = {
        .label          = label,
        .is_active      = false,
        .is_hot         = false,
        .hover_color    = DEFAULT_BUTTON_COLOR,
    };

    return button;
}
