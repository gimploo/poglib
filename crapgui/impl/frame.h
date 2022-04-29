#pragma once
#include "../decl.h"
#include "./button.h"
#include "./uielem.h"
#include "./label.h"

//TODO: MAKE A BATCH MANAGER URGENT !!!!!!!!!!!!!!!!!!!!!!

#define crapgui_get_font(PGUI, UITYPE)      &(PGUI)->fonts[UITYPE]

button_t *crapgui_get_button(const crapgui_t *gui, const char *frame_label, const char *button_label)
{
    assert(frame_label);
    assert(button_label);

    const map_t *map = &gui->frames;
    frame_t *frame = (frame_t *)map_get_value(map, frame_label);

    const slot_t *slot = &frame->uielems;
    slot_iterator(slot, iter) 
    {
        uielem_t *ui = (uielem_t *)iter;
        if (ui->type != UI_BUTTON) continue;
        
        if (strcmp(ui->title, button_label) == 0) 
            return &ui->button;
    }
    eprint("no button found");
}

void __frame_update(frame_t *frame, const crapgui_t *gui)
{
    // Updating the vertices for next frame
    /*if (!frame->__is_changed) return;*/

    // Clearing all batches
    for (u32 i = 0; i < MAX_UI_TYPE_ALLOWED_IN_FRAME; i++)
    {
        glbatch_t *uibatch = &frame->__uibatch[i];
        glbatch_t *txtbatch = &frame->__txtbatch[i];

        glbatch_clear(uibatch);
        glbatch_clear(txtbatch);
    }

    // Updating ui elements and adding its vertices to the batch
    slot_t *slot = &frame->uielems;
    slot_iterator(slot, iter) 
    {
        uielem_t *ui = (uielem_t *)iter;
        __uielem_update(ui, gui);

        glquad_t *uiquad    = &ui->glvertices;
        glbatch_t *uibatch  = &frame->__uibatch[ui->type];
        glbatch_put(uibatch, *uiquad);

        glbatch_t *txtbatch = &frame->__txtbatch[ui->type];
        glbatch_t *txtquads = &ui->textbatch;
        glbatch_combine(txtbatch, txtquads);
    }

    // Updating frame vertices
    frame->__vertices = quadf(vec3f(frame->pos), frame->dim.cmp[X], frame->dim.cmp[Y]);

    // Creating the framebuffer object texture
    {
        GL_CHECK(glClearColor(
                frame->color.cmp[0],
                frame->color.cmp[1],
                frame->color.cmp[2],
                frame->color.cmp[3]
        ));
        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        glframebuffer_t *fbo = &frame->__texture;
        glframebuffer_begin_texture(fbo);
            for (u32 i = 0; i < MAX_UI_TYPE_ALLOWED_IN_FRAME; i++)
            {
                glbatch_t *uibatch      = &frame->__uibatch[i]; 
                glbatch_t *txtbatch     = &frame->__txtbatch[i];

                if (glbatch_is_empty(uibatch)) {
                    continue;
                }

                glrenderer2d_t r2d = {
                    .shader = &gui->shaders[i],
                    .texture = NULL,
                };
                glrenderer2d_draw_from_batch(&r2d, uibatch);

                const glfreetypefont_t *font = crapgui_get_font(gui, i);
                glfreetypefont_draw(font, txtbatch);
            }
        glframebuffer_end_texture(fbo);
    }

    frame->__is_changed = false;
    frame->is_hot = false;
}

void __frame_render(const frame_t *frame, const crapgui_t *gui)
{
    glrenderer2d_t r2d = {
        .shader = &gui->shaders[UI_FRAME],
        .texture = &frame->__texture.texture2d,
    };

    glquad_t quad = glquad(
                frame->__vertices,
                frame->color,
                quadf(vec3f(0.0f), 1.0f, 1.0f), 
                0);

    glrenderer2d_draw_quad(&r2d, quad);
}

vec2f_t __frame_get_pos_for_new_uielem(const frame_t *frame)
{
    vec2f_t output          = {0};
    const slot_t *uielems   = &frame->uielems;
    const vec2f_t margin    = DEFAULT_UI_MARGIN;

    if (uielems->len == 0) 
        return (vec2f_t ) { -1.0f + margin.cmp[X], 1.0f - margin.cmp[Y] };

    uielem_t *prev_elem = (uielem_t *)slot_get_value(uielems, uielems->len - 1);
    assert(prev_elem);


    if ((prev_elem->pos.cmp[X] + (2.0f * prev_elem->dim.cmp[X]) + margin.cmp[X]) >= 1.0f) {

        output = (vec2f_t ){
            .cmp[X] = -1.0f + margin.cmp[X],
            .cmp[Y] = prev_elem->pos.cmp[Y] - prev_elem->dim.cmp[Y] - margin.cmp[Y],
        };

    } else {

        output = (vec2f_t ){
            .cmp[X] = prev_elem->pos.cmp[X] + prev_elem->dim.cmp[X] + margin.cmp[X],
            .cmp[Y] = prev_elem->pos.cmp[Y],
        };

    }

    if (output.cmp[Y] - prev_elem->dim.cmp[Y] - margin.cmp[Y] <= -1.0f)
        eprint("This frame cannot fit in the window");

    return output;
}


frame_t frame_init(const char *label, vec2f_t pos, vec4f_t color, vec2f_t dim)
{
    const u64 cap_per_batch = 
        MAX_UI_CAPACITY_PER_FRAME / MAX_UI_TYPE_ALLOWED_IN_FRAME;

    return (frame_t ) {
        .label              = label,
        .pos                = pos,
        .dim                = dim,
        .color              = color,
        .uielems            = slot_init(MAX_UI_CAPACITY_PER_FRAME, uielem_t ),
        .is_hot             = false,

        .__is_changed       = true,
        .__texture          = glframebuffer_init(global_window->width, global_window->height),

        .__uibatch            = {

            [UI_BUTTON] = glbatch_init(cap_per_batch, glquad_t ),
            [UI_LABEL]  = glbatch_init(cap_per_batch, glquad_t ),

        },
        .__txtbatch            = {

            [UI_BUTTON] = glbatch_init(cap_per_batch, glquad_t ),
            [UI_LABEL]  = glbatch_init(cap_per_batch, glquad_t ),

        },

        .update             = __frame_update,
        .render             = __frame_render
    };
}


void frame_destroy(frame_t *self)
{
    glframebuffer_destroy(&self->__texture);

    slot_t *slot = &self->uielems;
    slot_iterator(slot, iter) {

        uielem_t *elem = (uielem_t *)iter;
        __uielem_destroy(elem);
    }
    slot_destroy(slot);

    for (u32 i = 0; i < MAX_UI_TYPE_ALLOWED_IN_FRAME; i++)
    {
        glbatch_destroy(&self->__uibatch[i]);
        glbatch_destroy(&self->__txtbatch[i]);
    }

}


void __crapgui_frame_add_uielem(crapgui_t *gui, frame_t *frame, const char *label, uitype type)
{
    assert(type != UI_FRAME);

    vec2f_t uipos =__frame_get_pos_for_new_uielem(frame); 

    uielem_t tmp; 
    memset(&tmp, 0, sizeof(tmp));

    slot_t *slot = &frame->uielems;
    uielem_t *ui = (uielem_t *)slot_append(slot, tmp);

    switch(type)
    {
        case UI_BUTTON:
            ui->button       = button_init(ui);
            ui->dim          = DEFAULT_BUTTON_DIMENSIONS;
            ui->color        = DEFAULT_BUTTON_COLOR;
            ui->update       = __button_update;
        break;

        case UI_LABEL:
            ui->label    = label_init();
            ui->dim      = DEFAULT_LABEL_DIMENSIONS;
            ui->color    = DEFAULT_LABEL_COLOR;
            ui->update   = __label_update;
        break;

        default: eprint("type not accounted for ");
    }

    ui->title = label;
    ui->type = type;
    ui->pos  = uipos;
    ui->font = crapgui_get_font(gui, type);
    ui->textbatch = glbatch_init(KB/2, glquad_t );
    ui->is_changed = true;
        
}

