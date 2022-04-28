#pragma once
#include "../decl.h"
#include "./button.h"
#include "./uielem.h"
#include "./label.h"

//TODO: MAKE A BATCH MANAGER URGENT !!!!!!!!!!!!!!!!!!!!!!

#define frame_get_font(PFRAME, UITYPE)      &(PFRAME)->__gui->fonts[UITYPE]
#define frame_get_textbatch(PFRAME)         &(PFRAME)->__onlytextbatch

void __frame_update(frame_t *frame)
{
    crapgui_t *gui = frame->__gui;

    // Updating the vertices for next frame
    if (!frame->__is_changed) return;

    // Clearing all batches
    for (u32 i = 0; i < MAX_UI_TYPE_ALLOWED_IN_FRAME; i++)
    {
        glbatch_t *uibatch = &frame->__uibatch[i];
        glbatch_t *txtbatch = &frame->__txtbatch[i];

        glbatch_clear(uibatch);
        glbatch_clear(txtbatch);
    }

    // Updating ui elements and adding its vertices to the batch
    list_t *list = &frame->uielems;
    list_iterator(list, iter) 
    {
        uielem_t *ui = (uielem_t *)iter;
        __uielem_update(ui);

        glquad_t *uiquad      = &ui->__uivertices;
        glbatch_t *uibatch    = &frame->__uibatch[ui->type];
        glbatch_put(uibatch, *uiquad);

        glbatch_t *txtbatch = &frame->__txtbatch[ui->type];
        glbatch_t *txtquads = &ui->__textbatch;

        glbatch_combine(txtbatch, txtquads);
    }

    // Updating frame vertices
    frame->__vertices = glquad(
                quadf(vec3f(frame->pos), frame->dim.cmp[X], frame->dim.cmp[Y]), 
                frame->color,
                quadf(vec3f(0.0f), 1.0f, 1.0f), 
                0);

    // Creating the framebuffer object texture
    {
        GL_CHECK(glClearColor(
                frame->color.cmp[0],
                frame->color.cmp[1],
                frame->color.cmp[2],
                frame->color.cmp[3]
        ));

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

                glfreetypefont_t *font = frame_get_font(frame, i);
                glfreetypefont_draw(font, txtbatch);
            }
        glframebuffer_end_texture(fbo);
    }

    frame->__is_changed = false;
}

void __frame_render(frame_t *frame)
{
    crapgui_t *gui = (crapgui_t *)frame->__gui;

    glrenderer2d_t r2d = {
        .shader = &gui->shaders[UI_FRAME],
        .texture = &frame->__texture.texture2d,
    };

    glrenderer2d_draw_quad(&r2d, frame->__vertices);
}

vec2f_t __frame_get_pos_for_new_uielem(frame_t *frame)
{
    vec2f_t output          = {0};
    const list_t *uielems   = &frame->uielems;
    const vec2f_t margin    = DEFAULT_UI_MARGIN;

    if (uielems->len == 0) 
        return (vec2f_t ) { -1.0f + margin.cmp[X], 1.0f - margin.cmp[Y] };

    uielem_t *prev_elem = (uielem_t *)list_get_value(uielems, uielems->len - 1);
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


frame_t frame_init(crapgui_t *gui, const char *label, vec2f_t pos, vec4f_t color, vec2f_t dim)
{
    const u64 cap_per_batch = 
        MAX_UI_CAPACITY_PER_FRAME / MAX_UI_TYPE_ALLOWED_IN_FRAME;

    return (frame_t ) {
        .label              = label,
        .pos                = pos,
        .dim                = dim,
        .color              = color,
        .uielems            = list_init(MAX_UI_CAPACITY_PER_FRAME, uielem_t ),

        .__gui              = gui,
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

    list_t *list = &self->uielems;
    list_iterator(list, iter) {

        uielem_t *elem = (uielem_t *)iter;
        __uielem_destroy(elem);
    }
    list_destroy(list);

    for (u32 i = 0; i < MAX_UI_TYPE_ALLOWED_IN_FRAME; i++)
    {
        glbatch_destroy(&self->__uibatch[i]);
        glbatch_destroy(&self->__txtbatch[i]);
    }

}


void __frame_add_uielem(frame_t *frame, const char *label, uitype type)
{
    assert(type != UI_FRAME);

    uielem_t ui; memset(&ui, 0, sizeof(ui));

    ui.type           = type;
    ui.pos            = __frame_get_pos_for_new_uielem(frame);
    ui.__font         = frame_get_font(frame, type);
    ui.__is_changed   = true;
    ui.__textbatch    = glbatch_init(KB/2, glquad_t );


    switch(type)
    {
        case UI_BUTTON:{
            button_t o      = button_init(label);
            ui.dim          = DEFAULT_BUTTON_DIMENSIONS; 
            ui.color        = DEFAULT_BUTTON_COLOR;
            ui.update       = __button_update,
            ui.value        = (void *)calloc(1, sizeof(button_t ));
            memcpy(ui.value, &o, sizeof(o));
        } break;

        case UI_LABEL: {
            label_t o   = label_init(label);
            ui.dim      = DEFAULT_LABEL_DIMENSIONS;
            ui.color    = DEFAULT_LABEL_COLOR;
            ui.update   = __label_update;
            ui.value    = (void *)calloc(1, sizeof(label_t ));
            memcpy(ui.value, &o, sizeof(o));
        } break;

        default: eprint("type not accounted for ");
    }

    list_t *list = &frame->uielems;
    list_append(list, ui);
}

