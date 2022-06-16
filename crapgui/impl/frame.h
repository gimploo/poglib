#pragma once
#include "../decl.h"
#include "./ui.h"


void __frame_update(frame_t *frame, const crapgui_t *gui)
{
    // Updating frame vertices
    if (frame->__update_both_caches) {

        // The entire frame vertices
        frame->__frame_cache.quad = 
            quadf(vec3f(frame->pos), frame->styles.width, frame->styles.height);

        // frame header
        glbatch_clear(&frame->__frame_cache.glquads);
        glquad_t header = glquad(
                quadf((vec3f_t ){-1.0f, 1.0f, 1.0f}, 2.0f, 0.2f), 
                COLOR_BLACK, quadf(vec3f(0.0f),0.0f,0.0f), 0);
        glbatch_put(&frame->__frame_cache.glquads, header);

        gltext_clear(&frame->__frame_cache.texts); 
        glfreetypefont_add_text_to_batch(
                &gui->frame_assets.font,
                &frame->__frame_cache.texts.text, 
                frame->label, 
                (vec2f_t ) {-1.0f, 0.9f}, 
                COLOR_WHITE);

        frame->__update_both_caches = false;

    }

    if (!frame->uis.len) return;

    for (int type = 0; type < UITYPE_COUNT; type++)
    {
        glbatch_t   *uibatch  = &frame->__frame_ui_cache.uibatch[type];
        gltext_t    *txtbatch = &frame->__frame_ui_cache.txtbatch[type];
        glbatch_clear(uibatch);
        gltext_clear(txtbatch);
    }

    // Updating ui elements and adding its vertices to the batch
    slot_t *slot = &frame->uis;
    slot_iterator(slot, iter) 
    {
        ui_t *ui = (ui_t *)iter;
        glbatch_t   *uibatch  = &frame->__frame_ui_cache.uibatch[ui->type];
        glbatch_t   *txtbatch = &frame->__frame_ui_cache.txtbatch[ui->type].text;

        __ui_update(ui, frame, gui);

        glquad_t *uiquad = &ui->__cache.glquad;
        glbatch_put(uibatch, *uiquad);

        glbatch_t *txtquads = &ui->__cache.texts.text;
        glbatch_combine(txtbatch, txtquads);
    }

    GL_CHECK(glClearColor(
            frame->styles.color.cmp[0],
            frame->styles.color.cmp[1],
            frame->styles.color.cmp[2],
            frame->styles.color.cmp[3]
    ));
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    glframebuffer_t *fbo = &frame->__frame_ui_cache.texture;
    glframebuffer_begin_texture(fbo);

    // Texturing the uis together
    {
        for (u32 i = 0; i < UITYPE_COUNT; i++)
        {
            glbatch_t *uibatch      = &frame->__frame_ui_cache.uibatch[i]; 
            glbatch_t *txtbatch     = &frame->__frame_ui_cache.txtbatch[i].text;

            if (glbatch_is_empty(uibatch)) {
                continue;
            }

            glrenderer2d_t r2d = {
                .shader = &gui->ui_assets.shaders[i],
                .texture = NULL,
            };
            glrenderer2d_draw_from_batch(&r2d, uibatch);

            if (!glbatch_is_empty(txtbatch)) {
                const glfreetypefont_t *font = &gui->ui_assets.fonts[i];
                glfreetypefont_draw(font, txtbatch);
            }
        }

        // header
        glrenderer2d_t R2d = {
            .shader = &gui->__common_shader,
            .texture = NULL
        };
        glrenderer2d_draw_from_batch(&R2d, &frame->__frame_cache.glquads);
        glfreetypefont_draw(
                &gui->frame_assets.font,
                &frame->__frame_cache.texts.text);
    }
    glframebuffer_end_texture(fbo);
}

void __frame_render(const frame_t *frame, const crapgui_t *gui)
{
    glrenderer2d_t r2d = {
        .shader = &gui->frame_assets.shader,
        .texture = &frame->__frame_ui_cache.texture.texture2d,
    };

    glquad_t quad = {0};
    if (gui->edit_mode.is_on) {
        vec3f_t pos = {-1.0f, 1.0f, 0.0f};
        quad = glquad(
                    quadf(pos, 2.0f, 2.0f),
                    frame->styles.color,
                    quadf(vec3f(0.0f), 1.0f, 1.0f), 
                    0);
    } else {
        quad = glquad(
                    frame->__frame_cache.quad,
                    frame->styles.color,
                    quadf(vec3f(0.0f), 1.0f, 1.0f), 
                    0);
    }

    glrenderer2d_draw_quad(&r2d, quad);
}


//FIXME: the margin here is very wrong, because we take the positioning of the 
//prev element and then give a position that is besides it if the element 
//doesnt go out the frame, the y position is fixed and when we include margin 
//here we get a staircase like positioning, as of now i am thinking of having 
//an edit mode where u could drag the button and then place it where ever 
//u want this might be better idea than coding the styles. I am thinking the 
//after the editing is done, it saves all the stuff into a file. 
//This could be a better and easier approach than going the css route

vec2f_t __frame_get_pos_for_new_ui(const frame_t *frame, ui_t *ui)
{
    assert(ui);
    vec2f_t output          = {0};
    const slot_t *uis       = &frame->uis;

    //NOTE: since we add the ui to slot before calling this function, the previous logic 
    //we had wont work, ik i hate this, i guess i need to live with this for now
    if (uis->len == 1) 
        return (vec2f_t ) { -1.0f + frame->styles.margin.cmp[X] + ui->styles.margin.cmp[X], 1.0f - FRAME_HEADER_HEIGHT - frame->styles.margin.cmp[Y] - ui->styles.margin.cmp[Y] };

    ui_t *prev_elem = (ui_t *)slot_get_value(uis, uis->len - 2);
    assert(prev_elem);
    const vec2f_t prev_margin   = prev_elem->styles.margin;
    const quadf_t prev_rect     = prev_elem->__cache.quad;

    if ((prev_rect.vertex[TOP_RIGHT].cmp[X] + prev_margin.cmp[X] + ( 2.0f * ui->styles.margin.cmp[X]) + ui->styles.width) >= 1.0f) {

        output = (vec2f_t ){
            .cmp[X] = -1.0f + ui->styles.margin.cmp[X] + frame->styles.margin.cmp[X],
            .cmp[Y] = prev_rect.vertex[BOTTOM_LEFT].cmp[Y] - prev_margin.cmp[Y] - ui->styles.margin.cmp[Y],
        };

    } else {

        output = (vec2f_t ){
            .cmp[X] = prev_rect.vertex[TOP_RIGHT].cmp[X] + prev_margin.cmp[X] + ui->styles.margin.cmp[X],
            .cmp[Y] = prev_elem->pos.cmp[Y],
        };

    }

    if ((prev_rect.vertex[BOTTOM_LEFT].cmp[Y] - prev_margin.cmp[Y] - (2.0f * ui->styles.margin.cmp[Y]) + ui->styles.height) <= -1.0f)
        eprint("This ui cannot fit in the window");

    return output;
}


frame_t frame_init(const char *label, vec2f_t pos, uistyle_t styles)
{
    const u64 cap_per_batch = KB;

    return (frame_t ) {
        .label              = label,
        .pos                = pos,
        .styles             = styles,
        .uis                = slot_init(MAX_UI_CAPACITY_PER_FRAME, ui_t ),
        .__update_both_caches = true,

        .__frame_cache = {
            .quad                 = {0},
            .texts                 = gltext_init(cap_per_batch),
            .glquads              = glbatch_init(cap_per_batch, glquad_t ),
        },

        .__frame_ui_cache = {

            .texture = glframebuffer_init(global_window->width, global_window->height),
            .uibatch = {

                [UI_BUTTON] = glbatch_init(cap_per_batch, glquad_t ),
                [UI_LABEL]  = glbatch_init(cap_per_batch, glquad_t ),

            },
            .txtbatch = {

                [UI_BUTTON] = gltext_init(cap_per_batch),
                [UI_LABEL]  = gltext_init(cap_per_batch),

            },
        },

        .update             = __frame_update,
        .render             = __frame_render
    };
}


void frame_destroy(frame_t *self)
{

    slot_t *slot = &self->uis;
    slot_iterator(slot, iter) {

        ui_t *elem = (ui_t *)iter;
        __ui_destroy(elem);
    }
    slot_destroy(slot);

    for (u32 i = 0; i < UITYPE_COUNT; i++)
    {
        glbatch_destroy(&self->__frame_ui_cache.uibatch[i]);
        gltext_destroy(&self->__frame_ui_cache.txtbatch[i]);
    }

    glframebuffer_destroy(&self->__frame_ui_cache.texture);
    gltext_destroy(&self->__frame_cache.texts);
    glbatch_destroy(&self->__frame_cache.glquads);

}


//NOTE: this code took me 3 full days to make 💀
vec2f_t frame_get_mouse_position(const frame_t *frame)
{
    const vec2f_t mpos  = window_mouse_get_norm_position(global_window);
    const quadf_t rect  = frame->__frame_cache.quad;
    const vec2f_t dim_half = {
        frame->styles.width / 2.0f, 
        frame->styles.height / 2.0f 
    };

    const vec2f_t origin = {
        .cmp[X] = rect.vertex[TOP_LEFT].cmp[X] + (dim_half.cmp[X]),
        .cmp[Y] = rect.vertex[TOP_LEFT].cmp[Y] - (dim_half.cmp[Y])
    };

    const vec2f_t nmpos = {
        mpos.cmp[X] - origin.cmp[X],
        mpos.cmp[Y] - origin.cmp[Y],
    };

    return (vec2f_t ) {
        .cmp[X] = normalize(nmpos.cmp[X], 0.0f, dim_half.cmp[X]) ,
        .cmp[Y] = normalize(nmpos.cmp[Y], 0.0f, dim_half.cmp[Y]) 
    };
}

void __frame_add_ui(frame_t *frame, crapgui_t *gui, const char *label, uitype type, uistyle_t styles)
{
    ui_t tmp; 
    memset(&tmp, 0, sizeof(tmp));

    slot_t *slot = &frame->uis;
    ui_t *ui = (ui_t *)slot_append(slot, tmp);

    ui->styles          = styles;
    ui->styles.margin   = vec2f_add(styles.margin, frame->styles.padding);
    ui->type            = type;
    ui->title           = label;
    ui->is_active       = false;    
    ui->is_hot          = false;
    ui->pos             = __frame_get_pos_for_new_ui(
                            frame, ui);
    ui->__cache.texts   = gltext_init(KB);

    __ui_update(ui, frame, gui);
}
