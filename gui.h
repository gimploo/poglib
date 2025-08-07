#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/gfx/glrenderer3d.h>
#include <poglib/application.h>
#include <poglib/font/glfreetypefont.h>

/*
 * LAYOUT - origin is at the top left of the screen
 * Uses pixel units as measurements
 *
 *
 */

typedef enum ui_type {

    UI_TYPE_PANEL = 0,
    UI_TYPE_BUTTON,
    UI_TYPE_LABEL,
    UI_TYPE_CHECKBOX,
    UI_TYPE_RADIOBUTTON,
    UI_TYPE_SLIDER,

    UI_TYPE_COUNT

} ui_type;

typedef enum {
    UI_LAYOUT_VERTICAL = 0,
    UI_LAYOUT_HORIZONTAL = 1,
    UI_LAYOUT_COUNT
} ui_layout_type;

typedef struct {
    vec4f_t padding;
    vec4f_t margin;
    vec4f_t color;

    ui_layout_type layout;

    struct {
        u32 width; 
        u32 height;
    } dim;

} style_t;

typedef struct ui_t {

    str_t label;
    style_t style;
    ui_type type;

    struct {
        bool is_hot;
        bool is_clicked;
    } state;

    struct {
        vec2f_t pos;
        vec4f_t color;
        f32 zorder;
        bool is_dirty;
        struct { f32 width; f32 height; } dim;
        //TODO: cache the vertices
    } computed;

    const struct ui_t * const parent;
    list_t children; //ui_t *

} ui_t;

typedef enum {
    VTX_BUFFER_QUAD_INDEX = 0,
    VTX_BUFFER_TEXT_INDEX = 1,
    VTX_BUFFER_COUNT
} vtx_buffer_types;

typedef struct {
    ui_t *root;

    struct {
        list_t vtx[VTX_BUFFER_COUNT];
        list_t idx[VTX_BUFFER_COUNT];
        glshader_t shader;
    } gfx;

    struct {
        glfreetypefont_t handler;
        glshader_t custom_shader;
    } font;

    struct {
        list_t uistack;
        bool is_wireframe;
        bool is_dirty;
    } internals;

} gui_t;


/*======================================================
                    Declaration
======================================================*/

gui_t *     gui_init(void);
void        gui_set_wireframe_mode(gui_t *self, bool toggle);
void        gui_destroy(gui_t *self);

/*======================================================
                    Implemenentation
======================================================*/

vec4f_t __get_ui_padding(const ui_t *ui)
{
    return ui->type == UI_TYPE_LABEL ? vec4f(0.f) : ui->style.padding;
}


vec2f_t __accumulator_get_next_position_for_vertical_layout(const vec2f_t accum_pos, const ui_t *current_ui)
{
    const u32 pos_y = accum_pos.y + current_ui->computed.dim.height + (current_ui->style.margin.top - current_ui->style.margin.bottom);
    return (vec2f_t){
        .x = accum_pos.x,
        .y = pos_y
    };
}

vec2f_t __accumulator_get_next_position_for_horizontal_layout(const vec2f_t accum_pos, const ui_t *current_ui)
{
    const u32 pos_x = accum_pos.x + current_ui->computed.dim.width + (current_ui->style.margin.left - current_ui->style.margin.right); 
    return (vec2f_t) {
        .x = pos_x,
        .y = accum_pos.y
    };
}

vec2f_t __get_next_available_pos(const ui_t *parent)
{
    ASSERT(parent);
    vec2f_t pos = parent->computed.pos;
    list_iterator(&parent->children, child)
    {
        switch(parent->style.layout)
        {
            case UI_LAYOUT_VERTICAL:
                pos = __accumulator_get_next_position_for_vertical_layout(pos, child);
            break;
            case UI_LAYOUT_HORIZONTAL:
                pos = __accumulator_get_next_position_for_horizontal_layout(pos, child);
            break;
            default: eprint("Unfamiliar layout kind.");
        }
    }
    return pos;
}

f32 __get_text_total_glyph_width(const ui_t *ui, const gui_t *gui)
{
    f32 width = gui->font.handler.fontatlas[(u8)ui->label.data[0]].bw;
    for (u8 i = 0; i < ui->label.len; i++)
        width += gui->font.handler.fontatlas[(u8)ui->label.data[i]].bw;
    return width;
}

f32 __get_ui_width(const ui_t *ui, const gui_t *gui)
{
    if (ui->style.dim.width) 
        return ui->style.dim.width;
    const f32 final_padding_x  = max(__get_ui_padding(ui).left - __get_ui_padding(ui).right, 0);
    return (ui->type == UI_TYPE_BUTTON || ui->type == UI_TYPE_LABEL) 
        ? __get_text_total_glyph_width(ui, gui) + final_padding_x
        : 0;
}

f32 __get_ui_height(const ui_t *ui, const gui_t *gui)
{
    if (ui->style.dim.height) 
        return ui->style.dim.height;
    const f32 final_padding_y  = max(__get_ui_padding(ui).top - __get_ui_padding(ui).bottom, 0);
    return (ui->type == UI_TYPE_BUTTON || ui->type == UI_TYPE_LABEL) 
        ? final_padding_y + gui->font.handler.fontsize
        : 0;
}

ui_t * __ui_init(const ui_t *parent, const str_t label, const ui_type type, const style_t *style, gui_t *gui) 
{
    ui_t * ui = mem_init(&(ui_t) {
        .label = label,
        .type = type,
        .parent = parent,
        .children = list_init(ui_t *),
        .computed = {0},
        .style = style ? *style : (style_t){0},
        .state = {0}
    }, sizeof(ui_t));

    ui->computed.zorder = parent ? parent->computed.zorder + 0.2f : -1.0f;
    ui->computed.pos = parent ? __get_next_available_pos(parent) : (vec2f_t){0};
    ui->computed.dim.width = __get_ui_width(ui, gui);
    ui->computed.dim.height = __get_ui_height(ui, gui);

    return ui;
}

gui_t * gui_init(void)
{
    gui_t *gui = mem_init(&(gui_t) {
        .root = NULL,
        .gfx = {
            .vtx = {
               [VTX_BUFFER_QUAD_INDEX] = list_init(glquad_t),
               [VTX_BUFFER_TEXT_INDEX] = list_init(glquad_t),
            },
            .idx = {
               [VTX_BUFFER_QUAD_INDEX] = list_init(u32),
               [VTX_BUFFER_TEXT_INDEX] = list_init(u32),
            },
            .shader = glshader_from_file_init(
                "lib/poglib/gui/uishader.vs",
                "lib/poglib/gui/uishader.fs"
            )
        },
        .font = {
            .handler = glfreetypefont_init(DEFAULT_FONT_ROBOTO_MEDIUM_FILEPATH, 24, true),
            .custom_shader = glshader_from_file_init(
                "lib/poglib/gui/ui-text-shader.vs",
                "lib/poglib/gui/ui-text-shader.fs"
            )
        },
        .internals = {
            .uistack = list_init(ui_t *),
            .is_wireframe = false,
            .is_dirty = true
        }
    }, sizeof(gui_t));

    return gui;
}

void gui_set_wireframe_mode(gui_t *self, bool toggle)
{
    self->internals.is_wireframe = toggle;
}

vec3f_t __get_applied_styled_pos_outter(const ui_t *ui)
{
    const vec2f_t pos = ui->computed.pos;
    return (vec3f_t) {
        .x = pos.x + (ui->style.margin.left - ui->style.margin.right),
        .y = pos.y + (ui->style.margin.top - ui->style.margin.bottom),
        .z = ui->computed.zorder,
    };
}

vec3f_t __get_applied_padding_on_all_sides(const vec3f_t pos, const vec4f_t padding)
{
    return (vec3f_t) {
        .x = pos.x + padding.left - padding.right,
        .y = pos.y + padding.top - padding.bottom,
        .z = pos.z
    };
}

vec3f_t __get_applied_styled_pos_inner(const ui_t *ui)
{
    return __get_applied_padding_on_all_sides(
        __get_applied_styled_pos_outter(ui),
        ui->style.padding
    );
}

quadf_t __generate_ui_quad(const ui_t *ui, const gui_t *gui)
{
    return quadf_for_window_coordinates(
        __get_applied_styled_pos_outter(ui),
        ui->computed.dim.width,
        ui->computed.dim.height
    );
}

vec4f_t __get_ui_text_color(const ui_t *ui)
{
    if (!ui->style.color.r && !ui->style.color.g && !ui->style.color.b && !ui->style.color.a) return COLOR_BLACK;
    return ui->type == UI_TYPE_LABEL ? ui->style.color : COLOR_BLACK;
}


void __recache_ui_text(gui_t *gui, const ui_t *ui)
{
    if (ui->type == UI_TYPE_PANEL) return;

    list_t *vtxs = &gui->gfx.vtx[VTX_BUFFER_TEXT_INDEX];
    list_t *idxs = &gui->gfx.idx[VTX_BUFFER_TEXT_INDEX];

    const vec3f_t containerpos = __get_applied_styled_pos_outter(ui);
    vec3f_t textpos = { 
        containerpos.x, 
        containerpos.y, 
        ui->computed.zorder + 0.2f
    };

    for (u8 str_index = 0; str_index < ui->label.len; str_index++)
    {
        const f32 text_baseline = max(
            (containerpos.y + ui->computed.dim.height) - (textpos.y + gui->font.handler.fontatlas[(u8)ui->label.data[str_index]].bh), 
            0
        );
        const glquad_t quad = glfreetypefont_generate_glquad_for_char(
            &gui->font.handler,
            ui->label.data[str_index], 
            __get_applied_padding_on_all_sides(
                glms_vec3_add(textpos, (vec3f_t){ 0.f, text_baseline, 0.f }), 
                ui->style.padding
            ),
            __get_ui_text_color(ui)
        );

        textpos.x += gui->font.handler.fontatlas[(u8)ui->label.data[str_index]].ax;

        const u32 idx[] = GENERATE_QUAD_IDX(vtxs->len);

        list_append(vtxs, quad);
        list_append_multiple(idxs, idx);
    }
}

void __recache_ui_vtx(gui_t *gui, const ui_t *ui)
{
    if (ui->type == UI_TYPE_LABEL) return;

    list_t *vtxs = &gui->gfx.vtx[VTX_BUFFER_QUAD_INDEX];
    list_t *idxs = &gui->gfx.idx[VTX_BUFFER_QUAD_INDEX];

    glquad_t quad = glquad(
        __generate_ui_quad(ui, gui),
        ui->computed.color,
        (quadf_t){0}
    );

    const u32 idx[] = GENERATE_QUAD_IDX(vtxs->len);

    list_append(vtxs, quad);
    list_append_multiple(idxs, idx);
}

void __recache_ui(gui_t *gui, ui_t *ui, bool recache_text)
{
    if (!ui) return;

    ui->computed.is_dirty = false;

    if (ui->state.is_hot) {
        ui->computed.color = ui->style.color;
        ui->computed.color.a = 0.5f;
    } else {
        ui->computed.color = ui->style.color;
    }

    if (recache_text) {
        __recache_ui_text(gui, ui);
    }
    __recache_ui_vtx(gui, ui);

    list_iterator(&ui->children, child)
    {
        __recache_ui(gui, child, recache_text);
    }
}

void __recache_gui_vtx(gui_t *self, bool recache_text)
{
    list_clear(&self->gfx.vtx[VTX_BUFFER_QUAD_INDEX]);
    list_clear(&self->gfx.idx[VTX_BUFFER_QUAD_INDEX]);

    __recache_ui(self, self->root, recache_text);

    self->internals.is_dirty = false;
}

void __gui_render(gui_t *gui)
{
    if (gui->internals.is_dirty) {
        const bool recache_text = !gui->gfx.vtx[VTX_BUFFER_TEXT_INDEX].len;
        __recache_gui_vtx(gui, recache_text);
    }

    const gltexture2d_t *textures[1] = {
        &gui->font.handler.texture
    };

    glrenderer3d_draw((glrendererconfig_t){
        .calls = {
            .count = 2,
            .call = {
                [0] = {
                    .draw_mode = GL_TRIANGLES,
                    .is_wireframe = gui->internals.is_wireframe,
                    .vtx = {
                        .data = gui->gfx.vtx[VTX_BUFFER_QUAD_INDEX].data,
                        .size = list_get_size(&gui->gfx.vtx[VTX_BUFFER_QUAD_INDEX])
                    },
                    .idx = {
                        .data = gui->gfx.idx[VTX_BUFFER_QUAD_INDEX].data,
                        .nmemb = gui->gfx.idx[VTX_BUFFER_QUAD_INDEX].len
                    },
                    .textures = {0},
                    .attrs = {
                        .count = 2,
                        .attr = {
                            [0] = {
                                .type = GL_FLOAT,
                                .ncmp = 3,
                                .interleaved = {
                                    .offset = offsetof(glvertex2d_t, position),
                                    .stride = sizeof(glvertex2d_t)
                                },
                            },
                            [1] = {
                                .type = GL_FLOAT,
                                .ncmp = 4,
                                .interleaved = {
                                    .offset = offsetof(glvertex2d_t, color),
                                    .stride = sizeof(glvertex2d_t)
                                },
                            },
                        }
                    },
                    .shader_config = {
                        .shader = &gui->gfx.shader,
                        .uniforms = {
                            .count = 1,
                            .uniform = {
                                [0] = {
                                    .name = "projection",
                                    .type = "matrix4f_t",
                                    .value.mat4 = glms_ortho(0.0f, 1080.f, 920.f, 0.0f, -2.0f, 2.0f)
                                }
                            }
                        }
                    }
                },
                [1] = {
                    .vtx = {
                        .data = list_get_buffer(&gui->gfx.vtx[VTX_BUFFER_TEXT_INDEX]),
                        .size = list_get_size(&gui->gfx.vtx[VTX_BUFFER_TEXT_INDEX])
                    },
                    .idx = {
                        .data = list_get_buffer(&gui->gfx.idx[VTX_BUFFER_TEXT_INDEX]),
                        .nmemb = gui->gfx.idx[VTX_BUFFER_TEXT_INDEX].len
                    },
                    .textures = {
                        .count = 1,
                        .textures = textures,
                    },
                    .shader_config = {
                        .shader = &gui->font.custom_shader,
                        .uniforms = {
                            .count = 1,
                            .uniform = {
                                [0] = {
                                    .name = "projection",
                                    .type = "matrix4f_t",
                                    .value.mat4 = glms_ortho(0.0f, 1080.f, 920.f, 0.0f, -2.0f, 2.0f)
                                }
                            }
                        },
                    },
                    .is_wireframe = gui->internals.is_wireframe,
                    .attrs = {
                        .count = 3,
                        .attr = {
                            [0] = {
                                .type = GL_FLOAT,
                                .ncmp = 3,
                                .interleaved = {
                                    .offset = offsetof(glvertex2d_t, position),
                                    .stride = sizeof(glvertex2d_t)
                                },
                            },
                            [1] = {
                                .type = GL_FLOAT,
                                .ncmp = 4,
                                .interleaved = {
                                    .offset = offsetof(glvertex2d_t, color),
                                    .stride = sizeof(glvertex2d_t)
                                },
                            },
                            [2] = {
                                .type = GL_FLOAT,
                                .ncmp = 2,
                                .interleaved = {
                                    .offset = offsetof(glvertex2d_t, uv),
                                    .stride = sizeof(glvertex2d_t)
                                }
                            }
                        }
                    }
                }
            }
        }});
}

void __ui_update(gui_t *gui, ui_t *ui)
{
    ASSERT(gui);
    ASSERT(ui);

    if (ui->type == UI_TYPE_PANEL || ui->type == UI_TYPE_LABEL) return;

    const window_t *win = window_get_current_active_window();
    const vec2i_t mouse_pos = window_mouse_get_position(win);

    const bool is_cursor_on_ui = (f32)mouse_pos.x > ui->computed.pos.x
        && (f32)mouse_pos.x < ui->computed.pos.x + ui->computed.dim.width
        && (f32)mouse_pos.y > ui->computed.pos.y
        && (f32)mouse_pos.y < ui->computed.pos.y + ui->computed.dim.height;

    if (is_cursor_on_ui) {
        ui->state.is_hot = true; 
        ui->computed.is_dirty = true;
    } else if (!is_cursor_on_ui && ui->state.is_hot){
        ui->state.is_hot = false; 
        ui->computed.is_dirty = true;
    } else {
        ui->computed.is_dirty = false;
    }

    ui->state.is_clicked = is_cursor_on_ui && window_mouse_button_is_pressed(win, SDL_MOUSEBUTTON_LEFT);
    gui->internals.is_dirty = gui->internals.is_dirty || ui->computed.is_dirty;
}


void __ui_destroy(ui_t *ui)
{
    list_iterator(&ui->children, ui_elem) {
        __ui_destroy(ui_elem);
    }
    list_destroy(&ui->children);
    mem_free(ui, sizeof(ui_t));
}

void gui_destroy(gui_t *self)
{
    if (self->root) {
        __ui_destroy(self->root);
    }

    list_destroy(&self->internals.uistack);
    list_destroy(&self->gfx.vtx[VTX_BUFFER_QUAD_INDEX]);
    list_destroy(&self->gfx.idx[VTX_BUFFER_QUAD_INDEX]);
    list_destroy(&self->gfx.vtx[VTX_BUFFER_TEXT_INDEX]);
    list_destroy(&self->gfx.idx[VTX_BUFFER_TEXT_INDEX]);
    glshader_destroy(&self->gfx.shader);
    glshader_destroy(&self->font.custom_shader);
    glfreetypefont_destroy(&self->font.handler);
    mem_free(self, sizeof(gui_t));
}

const ui_t *__ui_already_exist(const ui_t *ui, const str_t label) 
{
    if (ui == NULL)                     return NULL;
    if (str_cmp(&ui->label, &label))    return ui;

    list_iterator(&ui->children, child) {
        const ui_t *ui_child = __ui_already_exist(child, label);
        if (ui_child != NULL) {
            return ui_child;
        }
    }
    return NULL;
}


const ui_t *__gui_add_ui(gui_t *gui, const str_t label, const ui_type type, const style_t style, list_t *gui_stack) 
{
    ASSERT(gui);
    if(gui->root == NULL && type != UI_TYPE_PANEL) {
        eprint("UI Panel is missing");
    }

    ui_t *parent = gui_stack->len 
        ? (ui_t *)list_get_value(gui_stack, gui_stack->len - 1) 
        : NULL;

    const ui_t *ui = __ui_already_exist(gui->root, label);
    if (!ui) {
        ui = __ui_init(parent, label, type, &style, gui);
        if (!gui->root) {
            gui->root = ui;
        }

        if (parent && type != UI_TYPE_PANEL) {
            list_append_ptr(&parent->children, ui);
        }
    }
    list_append_ptr(gui_stack, ui); 


    return ui;
}

#define WRAPPED_PLEX(TYPE, ...) (((struct { TYPE wrapped; }){ .wrapped = (__VA_ARGS__) }).wrapped)

#define __GEN_UI(UI_TYPE, LABEL, STYLE)\
    for(ui_t *LABEL = __gui_add_ui(PGUI, str(#LABEL), UI_TYPE, WRAPPED_PLEX(style_t, (STYLE)), __gui_stack);\
        LABEL;\
        __ui_update(PGUI, LABEL), list_delete(__gui_stack, __gui_stack->len - 1), LABEL = NULL)

#define GUI(PHANDLER)\
    for(bool __1 = true; __1;)\
        for(gui_t *PGUI = PHANDLER; __1; __gui_render(PHANDLER))\
            for(list_t *__gui_stack = &PGUI->internals.uistack; __1; __1 = false, list_clear(__gui_stack))

#define UI_PANEL(LABEL, STYLE)  __GEN_UI(UI_TYPE_PANEL, LABEL, STYLE)
#define UI_BUTTON(LABEL, STYLE) __GEN_UI(UI_TYPE_BUTTON, LABEL, STYLE)
#define UI_LABEL(LABEL, STYLE)  __GEN_UI(UI_TYPE_LABEL, LABEL, STYLE)

