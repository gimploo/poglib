#pragma once
#include "../decl.h"
#include "./button.h"


void __frame_update(frame_t *frame, crapgui_t *gui)
{
    // Updating the vertices for next frame
    if (!frame->__is_changed) return;

    for (u32 i = 0; i < MAX_UIELEM_TYPE_ALLOWED_IN_FRAME; i++)
    {
        glbatch_t *batch = &frame->__batch[i];
        glbatch_clear(batch);
    }

    list_t *list = &frame->uielems;
    list_iterator(list, iter) 
    {
        uielem_t *ui = (uielem_t *)iter;
        ui->update(ui);

        glquad_t *quad = &ui->__vertices;
        assert(quad);

        glbatch_t *batch = &frame->__batch[ui->type];
        glbatch_put(batch, *quad);
    }

    frame->__vertices = glquad(
                quadf(vec3f(frame->pos), frame->dim.cmp[X], frame->dim.cmp[Y]), 
                frame->color,
                quadf(vec3f(0.0f), 1.0f, 1.0f), 
                0);

    // Creating the texture
    glframebuffer_t *fbo = &frame->__texture;

    GL_CHECK(glClearColor(
            frame->color.cmp[0],
            frame->color.cmp[1],
            frame->color.cmp[2],
            frame->color.cmp[3]
    ));

    glframebuffer_begin_texture(fbo);
        for (u32 i = 0; i < MAX_UIELEM_TYPE_ALLOWED_IN_FRAME; i++)
        {
            glbatch_t *batch = &frame->__batch[i]; 
            if (glbatch_is_empty(batch)) {
                continue;
            }

            glrenderer2d_t r2d = {
                .shader = &gui->shaders[i],
                .texture = NULL,
            };

            glrenderer2d_draw_from_batch(&r2d, batch);
            printf("alskdfj\n");

        }
    glframebuffer_end_texture(fbo);


    frame->__is_changed = false;
}

void __frame_render(frame_t *frame, crapgui_t *gui)
{
    glrenderer2d_t r2d = {
        .shader = &gui->shaders[UIELEM_FRAME],
        .texture = &frame->__texture.texture2d,
    };

    glrenderer2d_draw_quad(&r2d, frame->__vertices);
}

vec2f_t __frame_get_pos_for_new_uielem(frame_t *frame)
{
    vec2f_t output          = {0};
    const list_t *uielems   = &frame->uielems;
    const vec2f_t margin    = DEFAULT_UIELEM_MARGIN;

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


frame_t frame_init(const char *label, vec2f_t pos, vec4f_t color, vec2f_t dim)
{
    //TODO: batch manager is in due

    const u64 cap_per_batch = 
        MAX_UIELEM_CAPACITY_PER_FRAME / MAX_UIELEM_TYPE_ALLOWED_IN_FRAME;

    return (frame_t ) {
        .label              = label,
        .pos                = pos,
        .dim                = dim,
        .color              = color,
        .uielems            = list_init(MAX_UIELEM_CAPACITY_PER_FRAME, uielem_t ),

        .__is_changed       = true,
        .__texture          = glframebuffer_init(global_window->width, global_window->height),

        .__batch            = {

                [UIELEM_BUTTON] = glbatch_init(cap_per_batch, glquad_t ),
        },

        .update             = __frame_update,
        .render             = __frame_render
    };
}

void __uielem_destroy(uielem_t *elem)
{
    free(elem->value);
    elem->value = NULL;
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

    for (u32 i = 0; i < MAX_UIELEM_TYPE_ALLOWED_IN_FRAME; i++)
    {
        glbatch_destroy(&self->__batch[i]);
    }
}

void __uielem_update(uielem_t *ui)
{
    switch(ui->type)
    {
        case UIELEM_BUTTON:{
            button_t *button = (button_t *)ui->value;
            if (ui->__is_changed)
                button->update(button, ui);
        } break;

        default: eprint("type not accounted for ");
    }
    ui->__is_changed = false;
}

void __frame_add_uielem(frame_t *frame, const char *label, uitype type)
{
    uielem_t ui = {
        .type           = type,
        .pos            = __frame_get_pos_for_new_uielem(frame),
        .__is_changed   = true,
        .update         = __uielem_update
    };
    switch(type)
    {
        case UIELEM_BUTTON:{
            button_t o  = button_init(label);
            ui.dim      = DEFAULT_BUTTON_DIMENSIONS; 
            ui.color    = DEFAULT_BUTTON_COLOR;
            ui.value    = (void *)calloc(1, sizeof(button_t ));
            memcpy(ui.value, &o, sizeof(o));
        } break;

        default: eprint("type not accounted for ");
    }

    list_t *list = &frame->uielems;
    list_append(list, ui);
}

