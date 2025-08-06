#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/gfx/glrenderer3d.h>
#include <poglib/application.h>

/*
 * LAYOUT - origin is at the top left of the screen
 *
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

typedef struct {
    vec4i_t padding;
    vec4i_t margin;
    vec4f_t color;

    union {
        vec2i_t vec;
        struct { u32 width; u32 height; } size;
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
        vec2i_t pos;
        vec4f_t color;
        f32 zorder;
        //TODO: cache the vertices
    } computed;

    const struct ui_t * const parent;
    list_t children; //ui_t *

} ui_t;

typedef struct {
    ui_t *root;
    struct {
        list_t vtx;
        list_t idx;
        glshader_t shader;
    } gfx;

    struct {
        list_t uistack;
    } internals;

} gui_t;


global gui_t *global_gui_instance = NULL;


/*======================================================
                    Declaration
======================================================*/

gui_t *     gui_init(void);
void        gui_destroy(gui_t *self);

/*======================================================
                    Implemenentation
======================================================*/

vec2i_t __compute_pos(const vec2i_t pos, const style_t style)
{
    return (vec2i_t) {
        .x = pos.x + style.margin.x,
        .y = pos.y + style.margin.y,
    };
}

ui_t * __ui_init(const ui_t *parent, const str_t label, const ui_type type, const style_t *style) 
{
    const vec2i_t prev_pos = parent ? parent->computed.pos : (vec2i_t){0};
    const f32 zorder = parent ? parent->computed.zorder + 0.2f : -1.0f;

    ui_t * ui =mem_init(&(ui_t) {
        .label = label,
        .type = type,
        .parent = parent,
        .children = list_init(ui_t *),
        .computed = {
            .pos = style ? __compute_pos(prev_pos, *style) : (vec2i_t){0},
            .zorder = zorder,
        },
        .style = style ? *style : (style_t){0},
        .state = {0}
    }, sizeof(ui_t));

    return ui;
}

gui_t * gui_init(void)
{
    if (global_gui_instance) {
        return global_gui_instance;
    }

    gui_t *gui = mem_init(&(gui_t) {
        .root = NULL,
        .gfx = {
            .vtx = list_init(glquad_t),
            .idx = list_init(u32),
            .shader = glshader_from_file_init(
                "lib/poglib/gui/uishader.vs",
                "lib/poglib/gui/uishader.fs"
            )
        },
        .internals = {
            .uistack = list_init(ui_t *)
        }
    }, sizeof(gui_t));

    global_gui_instance = gui;

    return gui;
}

quadf_t __generate_ui_quad(ui_t *ui)
{
    quadf_t output = {0};

    output.vertex[TOP_LEFT] = (vec3f_t) { 
        ui->computed.pos.x, 
        ui->computed.pos.y, 
        ui->computed.zorder 
    };
    output.vertex[TOP_RIGHT] = (vec3f_t) { 
        ui->computed.pos.x + ui->style.dim.size.width, 
        ui->computed.pos.y, 
        ui->computed.zorder 
    };
    output.vertex[BOTTOM_LEFT] = (vec3f_t) { 
        ui->computed.pos.x, 
        ui->computed.pos.y + ui->style.dim.size.height, 
        ui->computed.zorder 
    };
    output.vertex[BOTTOM_RIGHT] = (vec3f_t) { 
        ui->computed.pos.x + ui->style.dim.size.width, 
        ui->computed.pos.y + ui->style.dim.size.height, 
        ui->computed.zorder 
    };

    return output;
}

void __recache_ui_vtx(gui_t *gui, ui_t *ui)
{
    list_t *vtxs = &gui->gfx.vtx;
    list_t *idxs = &gui->gfx.idx;

    glquad_t quad = glquad(
        __generate_ui_quad(ui),
        ui->computed.color,
        (quadf_t){0}
    );

    const u32 idx[] = {
        [0] = 4 * vtxs->len + DEFAULT_QUAD_INDICES[0],
        [1] = 4 * vtxs->len + DEFAULT_QUAD_INDICES[1],
        [2] = 4 * vtxs->len + DEFAULT_QUAD_INDICES[2],
        [3] = 4 * vtxs->len + DEFAULT_QUAD_INDICES[3],
        [4] = 4 * vtxs->len + DEFAULT_QUAD_INDICES[4],
        [5] = 4 * vtxs->len + DEFAULT_QUAD_INDICES[5],
    };

    list_append(vtxs, quad);
    list_append_multiple(idxs, idx);

}

void __recache_ui(gui_t *gui, ui_t *ui)
{
    if (!ui) return;

    ui->computed.color = ui->style.color;

    __recache_ui_vtx(gui, ui);

    list_iterator(&ui->children, child)
    {
        __recache_ui(gui, child);
    }
}

void __recache_gui_vtx(gui_t *self)
{
    __recache_ui(self, self->root);
}

void __gui_render(gui_t *gui, const bool enabled_wireframe)
{
    if (!gui->gfx.vtx.len) {
        __recache_gui_vtx(gui);
    }

    glrenderer3d_draw((glrendererconfig_t){
        .calls = {
            .count = 1,
            .call = {
                [0] = {
                    .draw_mode = GL_TRIANGLES,
                    .is_wireframe = enabled_wireframe,
                    .vtx = {
                        .data = gui->gfx.vtx.data,
                        .size = list_get_size(&gui->gfx.vtx)
                    },
                    .idx = {
                        .data = gui->gfx.idx.data,
                        .nmemb = gui->gfx.idx.len
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
            }
        }
    }});
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
    list_destroy(&self->gfx.vtx);
    list_destroy(&self->gfx.idx);
    glshader_destroy(&self->gfx.shader);
    mem_free(self, sizeof(gui_t));
    global_gui_instance = NULL;
}

ui_t *__ui_already_exist(const ui_t *ui, const str_t label) 
{
    if (ui == NULL)                     return NULL;
    if (str_cmp(&ui->label, &label))    return ui;

    list_iterator(&ui->children, child) {
        ui_t *ui_child = __ui_already_exist(child, label);
        if (ui_child != NULL) {
            return ui_child;
        }
    }
    return NULL;
}


ui_t *__gui_add_ui(gui_t *gui, const str_t label, const ui_type type, const style_t style, list_t *gui_stack) 
{
    ASSERT(gui);
    if(gui->root == NULL && type != UI_TYPE_PANEL) {
        eprint("UI Panel is missing");
    }

    ui_t *parent = gui_stack->len 
        ? (ui_t *)list_get_value(gui_stack, gui_stack->len - 1) 
        : NULL;

    ui_t *ui = __ui_already_exist(gui->root, label);
    if (!ui) {
        ui = __ui_init(parent, label, type, &style);
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

#define GUI(PHANDLER)\
    for(bool __1 = true; __1;)\
        for(gui_t *PGUI = PHANDLER; __1; __gui_render(PHANDLER, false))\
            for(list_t *__gui_stack = &PGUI->internals.uistack; __1; __1 = false, list_clear(__gui_stack))

#define UI_PANEL(LABEL, STYLE)\
    for(ui_t *LABEL = __gui_add_ui(PGUI, str(#LABEL), UI_TYPE_PANEL, WRAPPED_PLEX(style_t, (STYLE)), __gui_stack);\
        LABEL;\
        list_delete(__gui_stack, __gui_stack->len - 1), LABEL = NULL)

#define UI_BUTTON(LABEL, STYLE)\
    for(ui_t *LABEL = __gui_add_ui(PGUI, str(#LABEL), UI_TYPE_BUTTON, WRAPPED_PLEX(style_t, (STYLE)), __gui_stack);\
        LABEL;\
        list_delete(__gui_stack, __gui_stack->len - 1), LABEL = NULL)

#define UI_LABEL(LABEL, STYLE)\
    for(ui_t *LABEL = __gui_add_ui(PGUI, str(#LABEL), UI_TYPE_LABEL, WRAPPED_PLEX(style_t, (STYLE)), __gui_stack);\
        LABEL;\
        list_delete(__gui_stack, __gui_stack->len - 1), LABEL = NULL)

