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

    style_t style;
    ui_type type;

    struct {
        bool is_hot;
        bool is_clicked;
    } state;

    struct {
        vec2i_t pos;
        vec4f_t color;
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
} gui_t;

static gui_t *global_gui = NULL;



/*======================================================
                    Declaration
======================================================*/

gui_t * gui_init(void);
void gui_render(gui_t *gui, const bool enabled_wireframe);
void gui_destroy(gui_t *self);

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

ui_t * __ui_init(const ui_t *parent, const ui_type type, const style_t *style) 
{
    const vec2i_t prev_pos = parent ? parent->computed.pos : (vec2i_t){0};

    ui_t * ui =mem_init(&(ui_t) {
        .type = type,
        .parent = parent,
        .children = list_init(ui_t *),
        .computed = {
            .pos = style ? __compute_pos(prev_pos, *style) : (vec2i_t){0}
        },
        .style = style ? *style : (style_t){0},
        .state = {0}
    }, sizeof(ui_t));

    return ui;
}

gui_t * gui_init(void)
{
    if(global_gui) {
        eprint("Already another gui instance is running, multiple instances are not supported.");
    }

    ui_t *panel_ui = __ui_init(
        NULL, 
        UI_TYPE_PANEL, 
        &(const style_t ){
            .color = COLOR_BLACK, 
            .dim = { 20, 30 },
            .margin = { 9, 9 },
            .padding = {0}
        }
    );

    gui_t *gui = mem_init(&(gui_t) {
        .root = panel_ui,
        .gfx = {
            .vtx = list_init(glquad_t),
            .idx = list_init(u32),
            .shader = glshader_from_file_init(
                "lib/poglib/gui/uishader.vs",
                "lib/poglib/gui/uishader.fs"
            )
        }
    }, sizeof(gui_t));

    global_gui = gui;

    return global_gui;
}

vec3f_t __get_ndc_position(const ui_t *ui) { 

    const window_t *win = global_window;
    vec3f_t result = {0, 0, -1.f};

    // Normalize UI coordinates to [0, 1] range
    f32 x_normalized = ui->computed.pos.x / (f32)win->width;
    f32 y_normalized = ui->computed.pos.y / (f32)win->height;

    // Map to OpenGL NDC: [-1, 1] range, flip y-axis
    result.x = 2.0f * x_normalized - 1.0f; // Maps [0, 1] to [-1, 1]
    result.y = 1.0f - 2.0f * y_normalized; // Maps [0, 1] to [1, -1], flipping y

    return result;
}

void __recache_ui_vtx(gui_t *gui, ui_t *ui)
{
    list_t *vtxs = &gui->gfx.vtx;
    list_t *idxs = &gui->gfx.idx;

    vec3f_t dim_norm = glms_normalize((vec3f_t){
        .x = (f32)ui->style.dim.vec.x,
        .y = (f32)ui->style.dim.vec.y,
        .z = 0.f
    });

    glquad_t quad = glquad(
        quadf(
            __get_ndc_position(ui), 
            dim_norm.x, 
            dim_norm.y
        ),
        ui->computed.color,
        (quadf_t){0}
    );

    const u32 idx[] = {
        [0] = vtxs->len + DEFAULT_QUAD_INDICES[0],
        [1] = vtxs->len + DEFAULT_QUAD_INDICES[1],
        [2] = vtxs->len + DEFAULT_QUAD_INDICES[2],
        [3] = vtxs->len + DEFAULT_QUAD_INDICES[3],
        [4] = vtxs->len + DEFAULT_QUAD_INDICES[4],
        [5] = vtxs->len + DEFAULT_QUAD_INDICES[5],
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

void gui_render(gui_t *gui, const bool enabled_wireframe)
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
                        .uniforms = {0}
                    }
                }
            }
        }

    });
}


void __ui_destroy(ui_t *ui)
{
    if (ui->parent) {
        mem_free(ui->parent, sizeof(ui_t));
    }

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

    list_destroy(&self->gfx.vtx);
    list_destroy(&self->gfx.idx);
    glshader_destroy(&self->gfx.shader);
    mem_free(self, sizeof(gui_t));
    global_gui = NULL;
}



/*
 * GUI {
    PANEL({}) {
        ui_t *ui = BUTTON({});
        if (ui->ishot) {

        }

        BUTTON({}) {
            LABEL();
        };
        BUTTON({});
    }
 }
*/


ui_t *__gui_add_ui(ui_t *parent, ui_type type, const style_t style) 
{
    ui_t *ui = __ui_init(parent, type, &style);
    list_append_ptr(parent->children, ui);
    return ui;
}

#define GUI\
    list_t __ui_stack = list_init(ui_t *);\
    list_append(&__ui_stack, global_gui->root);\
    void *__ui_current_context = list_get_value(&__ui_stack, __ui_stack.len - 1);\
    for(ui_t *__ui_parent = __ui_current_context; __ui_stack.len; list_destroy(&__ui_stack))

#define UI_BUTTON(...)\
    list_append_ptr(&__ui_stack, __gui_add_ui(__ui_parent, UI_TYPE_BUTTON, __VA_ARGS__ ));\
    __ui_current_context = list_get_value(&__ui_stack, __ui_stack.len - 1);\
    for(__ui_parent = __ui_current_context; \
        __ui_parent != __ui_current_context;\
        list_delete(&__ui_stack, __ui_stack.len - 1), __ui_parent = list_get_value(&__ui_stack, __ui_stack.len - 1))

#define UI_PANEL(...)\
    list_append_ptr(&__ui_stack, __gui_add_ui(__ui_parent, UI_TYPE_PANEL, __VA_ARGS__ ));\
    __ui_current_context = list_get_value(&__ui_stack, __ui_stack.len - 1);\
    for(__ui_parent = __ui_current_context; \
        __ui_parent != __ui_current_context;\
        list_delete(&__ui_stack, __ui_stack.len - 1), __ui_parent = list_get_value(&__ui_stack, __ui_stack.len - 1))

