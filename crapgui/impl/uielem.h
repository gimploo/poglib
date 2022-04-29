#pragma once
#include "../decl.h"

typedef struct uielem_t {

    union {
        button_t    button;
        label_t     label;
    };
    uitype              type;
    const char          *title;
    vec2f_t             pos;
    vec2f_t             dim;
    vec4f_t             color;
    glfreetypefont_t    *font;
    bool                is_changed;
    glquad_t            glvertices;
    glbatch_t           textbatch;

    void (*update)(struct uielem_t *, const crapgui_t *gui);

} uielem_t ;

void __uielem_destroy(uielem_t *elem)
{
    elem->font = NULL;
    glbatch_destroy(&elem->textbatch);
}
void __uielem_update(uielem_t *ui, const crapgui_t *gui)
{
    if (!ui->is_changed) return;

    printf("UPDATING UI (%i)\n", ui->type);
    ui->update(ui, gui);
    ui->is_changed = false;
}
