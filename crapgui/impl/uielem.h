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
    vec2f_t             margin;
    vec4f_t             color;
    bool                is_changed;
    glquad_t            glvertices;
    glbatch_t           textbatch;
    quadf_t             vertices;
    glfreetypefont_t    *font;
    frame_t             *frame;

    void (*update_ui)(struct uielem_t *);

} uielem_t ;

uielem_t * __uielem_get_reference_toitself_in_frame(const frame_t *frame, const char *label, const uitype type)
{
    const slot_t *slot = &frame->uielems;
    slot_iterator(slot, iter) {
        uielem_t *ui = (uielem_t *)iter;
        if (ui->type != type) continue;

        if (strcmp(label, ui->title) == 0) return ui;
    }

    eprint("ui `%s` not found ", label);
}

void __uielem_destroy(uielem_t *elem)
{
    elem->font = NULL;
    glbatch_destroy(&elem->textbatch);
}

//TODO: stopped here last 

void __uielem_update(uielem_t *ui, const frame_t *frame)
{
    ui->update_ui(ui);

    if (!ui->is_changed) 
        return;

    //TODO: only recache if it changed position, better use uniforms for color changes
    /*printf("Recaching UI (%s)\n", ui->title);*/

    ui->vertices = 
        quadf(vec3f(ui->pos), ui->dim.cmp[X], ui->dim.cmp[Y]);

    ui->glvertices = 
        glquad(ui->vertices,
                ui->color,
                quadf(vec3f(0.0f), 0.0f, 0.0f), 0);


    /*quadf_print(ui->frame_relative_vertices);*/
    /*exit(1);*/

    glbatch_t *txtbatch = &ui->textbatch;

    vec2f_t centerpos = {
        .cmp[X] = ui->pos.cmp[X] ,
        .cmp[Y] = ui->pos.cmp[Y] - ui->dim.cmp[Y]/2
    };

    glfreetypefont_t *font = ui->font;
    glbatch_clear(txtbatch);
    glfreetypefont_add_text_to_batch(
            font, txtbatch, ui->title, centerpos, COLOR_WHITE);

    ui->is_changed = false;
}
