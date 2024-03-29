#pragma once
#include "../decl.h"
#include "./ui.h"

void __frame_update(frame_t *frame, crapgui_t *gui)
{
    // Updating frame vertices
    if (frame->__cache.self.cache_again) 
    {
        /*printf("Caching only frame %s\n", frame->label);*/
        // The entire frame vertices
        frame->__cache.self.quad = quadf(
                                (vec3f_t ){ frame->pos.x, frame->pos.y, 0.0f} , 
                                frame->styles.width, 
                                frame->styles.height);

        // frame header
        glbatch_clear(&frame->__cache.self.glquads);
        gltext_clear(&frame->__cache.self.texts); 

        glquad_t header = glquad(
                quadf((vec3f_t ){-1.0f, 1.0f, 1.0f}, 2.0f, 0.2f), 
                COLOR_BLACK, quadf(vec3f(0.0f),1.0f,1.0f));

        glbatch_put(&frame->__cache.self.glquads, header);

        glfreetypefont_add_text_to_batch(
                &gui->frame_assets.font,
                &frame->__cache.self.texts.text, 
                frame->label, 
                (vec2f_t ) {-1.0f, 0.9f}, 
                COLOR_WHITE);

        frame->__cache.self.cache_again = false;
    }

    if (!frame->__cache.uis.cache_again) return;
    /*printf("Caching all uis in frame %s\n", frame->label);*/

    for (int type = 0; type < UITYPE_COUNT; type++)
    {
        glbatch_t   *uibatch  = &frame->__cache.uis.uibatch[type];
        gltext_t    *txtbatch = &frame->__cache.uis.txtbatch[type];
        glbatch_clear(uibatch);
        gltext_clear(txtbatch);
    }

    // Updating ui elements and adding its vertices to the batch
    slot_t *slot = &frame->uis;
    slot_iterator(slot, iter) 
    {
        ui_t *ui = (ui_t *)iter;
        glbatch_t   *uibatch  = &frame->__cache.uis.uibatch[ui->type];
        glbatch_t   *txtbatch = &frame->__cache.uis.txtbatch[ui->type].text;

        /*printf("Frame is recaching ui ->    %s\n", ui->title);*/
        __ui_update(ui, frame, gui);

        glquad_t *uiquad = &ui->__cache.glquad;
        glbatch_put(uibatch, *uiquad);

        glbatch_t *txtquads = &ui->__cache.texts.text;
        glbatch_combine(txtbatch, txtquads);
    }

    GL_CHECK(glClearColor(
            frame->styles.color.r,
            frame->styles.color.g,
            frame->styles.color.b,
            frame->styles.color.a
    ));
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Texturing the uis together
    glframebuffer_begin_texture(&frame->__cache.uis.texture);
    {
        for (u32 i = 0; i < UITYPE_COUNT; i++)
        {
            glbatch_t *uibatch      = &frame->__cache.uis.uibatch[i]; 
            glbatch_t *txtbatch     = &frame->__cache.uis.txtbatch[i].text;

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
        glrenderer2d_draw_from_batch(&R2d, &frame->__cache.self.glquads);
        glfreetypefont_draw(
                &gui->frame_assets.font,
                &frame->__cache.self.texts.text);
    }
    glframebuffer_end_texture(&frame->__cache.uis.texture);

    frame->__cache.uis.cache_again = false;
}

void __frame_render(const frame_t *frame, const crapgui_t *gui)
{
    glrenderer2d_t r2d = {
        .shader = &gui->frame_assets.shader,
        .texture = &frame->__cache.uis.texture.texture2d,
    };
    glquad_t quad = glquad(
                frame->__cache.self.quad,
                frame->styles.color,
                quadf(vec3f(0.0f), 1.0f, 1.0f));
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
        return (vec2f_t ) { -1.0f + frame->styles.margin.x + ui->styles.margin.x, 1.0f - FRAME_HEADER_HEIGHT - frame->styles.margin.y - ui->styles.margin.y };

    ui_t *prev_elem = (ui_t *)slot_get_value(uis, uis->len - 2);
    assert(prev_elem);
    const vec2f_t prev_margin   = prev_elem->styles.margin;
    const quadf_t prev_rect     = prev_elem->__cache.quad;

    if ((prev_rect.vertex[TOP_RIGHT].x + prev_margin.x + ( 2.0f * ui->styles.margin.x) + ui->styles.width) >= 1.0f) {

        output = (vec2f_t ){
            .x = -1.0f + ui->styles.margin.x + frame->styles.margin.x,
            .y = prev_rect.vertex[BOTTOM_LEFT].y - prev_margin.y - ui->styles.margin.y,
        };

    } else {

        output = (vec2f_t ){
            .x = prev_rect.vertex[TOP_RIGHT].x + prev_margin.x + ui->styles.margin.x,
            .y = prev_elem->pos.y,
        };

    }

    if ((prev_rect.vertex[BOTTOM_LEFT].y - prev_margin.y - (2.0f * ui->styles.margin.y) + ui->styles.height) <= -1.0f)
        eprint("This ui cannot fit in the window");

    return output;
}

__frame_cache_t __frame_cache_init(void)
{
    return (__frame_cache_t ) {
            .self = {
                .cache_again    = true,
                .quad           = {0},
                .texts          = gltext_init(KB),
                .glquads        = glbatch_init(KB, glquad_t ),
            },
            .uis = {
                .cache_again    = false,
                .texture        = glframebuffer_init(global_window->width, global_window->height),
                .uibatch = {

                    [UI_BUTTON] = glbatch_init(KB, glquad_t ),
                    [UI_LABEL]  = glbatch_init(KB, glquad_t ),

                },
                .txtbatch = {

                    [UI_BUTTON] = gltext_init(KB),
                    [UI_LABEL]  = gltext_init(KB),

                },
            },
    };
}

frame_t frame_init(const char *label, vec2f_t pos, uistyle_t styles)
{
    assert(strlen(label) < 16 );
    frame_t o = {
        .label              = {0},
        .pos                = pos,
        .styles             = styles,
        .uis                = slot_init(MAX_UI_CAPACITY_PER_FRAME, ui_t ),
        .__cache            = __frame_cache_init(), 
    };

    memcpy(o.label, label, sizeof(o.label));
    return o;
}


void frame_destroy(frame_t *self)
{
    slot_t *uis = &self->uis;
    slot_iterator(uis, iter) {

        ui_t *ui = (ui_t *)iter;
        __ui_destroy(ui);
    }
    slot_destroy(uis);

    for (u32 i = 0; i < UITYPE_COUNT; i++)
    {
        glbatch_destroy(&self->__cache.uis.uibatch[i]);
        gltext_destroy(&self->__cache.uis.txtbatch[i]);
    }

    glframebuffer_destroy(&self->__cache.uis.texture);
    gltext_destroy(&self->__cache.self.texts);
    glbatch_destroy(&self->__cache.self.glquads);

}


//NOTE: this code took me 3 full days to make 💀
vec2f_t frame_get_mouse_position(const frame_t *frame)
{
    const vec2f_t mpos  = window_mouse_get_norm_position(global_window);
    const quadf_t rect  = frame->__cache.self.quad;
    const vec2f_t dim_half = {
        frame->styles.width / 2.0f, 
        frame->styles.height / 2.0f 
    };

    const vec2f_t origin = {
        .x = rect.vertex[TOP_LEFT].x + (dim_half.x),
        .y = rect.vertex[TOP_LEFT].y - (dim_half.y)
    };

    const vec2f_t nmpos = {
        mpos.x - origin.x,
        mpos.y - origin.y,
    };

    return (vec2f_t ) {
        .x = normalize(nmpos.x, 0.0f, dim_half.x) ,
        .y = normalize(nmpos.y, 0.0f, dim_half.y) 
    };
}


void __frame_add_ui(
        frame_t     *frame, 
        crapgui_t   *gui, 
        const char  *label, 
        uitype      type, 
        uistyle_t   styles)
{
    assert(strlen(label) < 16 );
    frame->__cache.uis.cache_again = true;

    ui_t tmp = {0}; 
    memset(&tmp, 0, sizeof(tmp));

    slot_t *slot = &frame->uis;
    ui_t *ui            = (ui_t *)slot_append(slot, tmp);

    {
        tmp = __ui_init(label, type, styles);
        *ui = tmp;
    }
    ui->pos             = __frame_get_pos_for_new_ui(frame, ui);
    ui->styles.margin   = glms_vec2_add(styles.margin, frame->styles.padding);

    // NOTE: dont move this anywhere else
    __ui_update(ui, frame, gui);
}
