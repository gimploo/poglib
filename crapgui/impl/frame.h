#pragma once
#include "../decl.h"
#include "./uielem.h"

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
    if (!frame->__is_changed) return;

    /*printf("Frame %s updating\n", frame->label);*/

    // Clearing all batches
    for (u32 i = 0; i < MAX_UI_TYPE_ALLOWED_IN_FRAME; i++)
    {
        glbatch_t *uibatch = &frame->__uibatch[i];
        glbatch_t *txtbatch = &frame->__txtbatch[i];

        glbatch_clear(uibatch);
        glbatch_clear(txtbatch);
    }

    // Updating frame vertices
    frame->__vertices = 
        quadf(vec3f(frame->pos), frame->dim.cmp[X], frame->dim.cmp[Y]);

    // Updating ui elements and adding its vertices to the batch
    slot_t *slot = &frame->uielems;
    slot_iterator(slot, iter) 
    {
        uielem_t *ui = (uielem_t *)iter;
        __uielem_update(ui, frame);

        glquad_t *uiquad    = &ui->glvertices;
        glbatch_t *uibatch  = &frame->__uibatch[ui->type];
        glbatch_put(uibatch, *uiquad);

        glbatch_t *txtbatch = &frame->__txtbatch[ui->type];
        glbatch_t *txtquads = &ui->textbatch;
        glbatch_combine(txtbatch, txtquads);
    }


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

                if (!glbatch_is_empty(txtbatch)) {
                    const glfreetypefont_t *font = crapgui_get_font(gui, i);
                    glfreetypefont_draw(font, txtbatch);
                }
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
    /*vec3f_t pos = {-1.0f, 1.0f, 0.0f};*/
    /*glquad_t quad = glquad(*/
                /*quadf(pos, 2.0f, 2.0f),*/
                /*frame->color,*/
                /*quadf(vec3f(0.0f), 1.0f, 1.0f), */
                /*0);*/

    glrenderer2d_draw_quad(&r2d, quad);
}


vec2f_t __frame_get_pos_for_new_uielem(const frame_t *frame, const vec2f_t ndim)
{
    vec2f_t output          = {0};
    const slot_t *uielems   = &frame->uielems;
    const vec2f_t dfmargin    = DEFAULT_UI_MARGIN;

    //NOTE: since we add the ui to slot before calling this function, the previous logic 
    //we had wont work, ik i hate this, i guess i need to live with this for now
    if (uielems->len == 1) 
        return (vec2f_t ) { -1.0f + dfmargin.cmp[X], 1.0f - dfmargin.cmp[Y] };

    uielem_t *prev_elem = (uielem_t *)slot_get_value(uielems, uielems->len - 2);
    assert(prev_elem);
    const vec2f_t margin = prev_elem->margin;

    if ((prev_elem->pos.cmp[X] + (prev_elem->dim.cmp[X] + ndim.cmp[X]) + margin.cmp[X]) >= 1.0f) {

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

    if (output.cmp[Y] - prev_elem->dim.cmp[Y] - ndim.cmp[Y] - margin.cmp[Y] <= -1.0f)
        eprint("This uielem cannot fit in the window");

    return output;
}


frame_t frame_init(const char *label, vec2f_t pos, vec4f_t color, vec2f_t dim)
{
    const u64 cap_per_batch = KB;

    return (frame_t ) {
        .label              = label,
        .pos                = pos,
        .dim                = dim,
        .color              = color,
        .margin             = DEFAULT_FRAME_MARGIN,
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

//NOTE: this code took me 3 full days to figure out 💀
vec2f_t __frame_get_mouse_position(const frame_t *frame)
{
    const vec2f_t mpos  = window_mouse_get_norm_position(global_window);
    const quadf_t rect  = frame->__vertices;
    const vec2f_t dim   = frame->dim;

    const vec2f_t origin = {
        .cmp[X] = rect.vertex[TOP_LEFT].cmp[X] + (dim.cmp[X]/2),
        .cmp[Y] = rect.vertex[TOP_LEFT].cmp[Y] - (dim.cmp[Y]/2)
    };

    const vec2f_t nmpos = {
        mpos.cmp[X] - origin.cmp[X],
        mpos.cmp[Y] - origin.cmp[Y],
    };

    return (vec2f_t ) {
        .cmp[X] = normalize(nmpos.cmp[X], 0.0f, 0.4f) ,
        .cmp[Y] = normalize(nmpos.cmp[Y], 0.0f, 0.4f) 
    };
}



