#pragma once
#include "../decl.h"
#include "./uielem.h"

void __label_update(uielem_t *uielem, const crapgui_t *gui)
{
    (void)gui;

    label_t *label = &uielem->label;

    uielem->glvertices = 
        glquad(quadf(vec3f(uielem->pos), uielem->dim.cmp[X], uielem->dim.cmp[Y]),
                uielem->color,
                quadf(vec3f(0.0f), 0.0f, 0.0f), 0);


    glbatch_clear(&uielem->textbatch);
    vec2f_t centerpos = {
        .cmp[X] = uielem->pos.cmp[X],
        .cmp[Y] = uielem->pos.cmp[Y] - uielem->dim.cmp[Y]
    };
    glfreetypefont_add_text_to_batch(
            uielem->font, &uielem->textbatch, 
            uielem->title, centerpos, label->textcolor);
}

label_t label_init(void)
{
    return (label_t ) {
        .textcolor = COLOR_WHITE
    };
}
