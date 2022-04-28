#pragma once
#include "../decl.h"

void __label_update(uielem_t *uielem)
{
    label_t *label = (label_t *)uielem->value;

    uielem->__uivertices = 
        glquad(quadf(vec3f(uielem->pos), uielem->dim.cmp[X], uielem->dim.cmp[Y]),
                uielem->color,
                quadf(vec3f(0.0f), 0.0f, 0.0f), 0);

    glbatch_clear(&uielem->__textbatch);
    vec2f_t centerpos = {
        .cmp[X] = uielem->pos.cmp[X],
        .cmp[Y] = uielem->pos.cmp[Y] - uielem->dim.cmp[Y]
    };
    glfreetypefont_add_text_to_batch(
            uielem->__font, &uielem->__textbatch, 
            label->label, centerpos, COLOR_WHITE);
}

label_t label_init(const char *label)
{
    return (label_t ) {
        .label = label
    };
}
