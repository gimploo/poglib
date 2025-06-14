#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/gfx/glrenderer3d.h>

typedef enum ui_type {
    UI_NONE = 0,
    UI_PANEL,
    UI_BUTTON,
    UI_LABEL,
    UI_SLIDER,
} ui_type;

typedef struct {

    // NOTE: 
    //Either pass for left/right and top/bottom i.e as right has more 
    //precedence over left similar is for bottom over top
    vec4f_t margin;

    vec2f_t dim;

} style_t;

typedef struct ui_t {

    ui_type type;
    bool is_dirty;

    struct {

        struct {
            bool is_hovered;
            bool is_clicked;
        } state;

        str_t label;
        style_t style;

    } data;

    struct ui_t *left; 
    struct ui_t *right; 

} ui_t;

typedef struct {

    ui_t *root; 

    struct {
        glshader_t shader;
        list_t vtx;
        list_t idx;
    } gfx;

} gui_t;


gui_t gui_init(style_t style);
void gui_render(gui_t *self, const bool in_wireframe);
void gui_destroy(gui_t *self);


vec2f_t __calculate_position_from_margin_and_width(style_t style)
{
    vec2f_t pos = {0.0f, 0.0f};
    vec4f_t margin = style.margin;
    vec2f_t dim = style.dim;

    // ----- horizontal (x) -----
    if (margin.left != 0.0f) {
        // anchored to left edge (‑1), move rightward by margin.l
        pos.x = -1.0f + margin.left;
    } else if (margin.right != 0.0f) {
        // anchored to right edge (+1), move leftward by margin.r and width
        pos.x =  1.0f - margin.right - dim.x;
    } else {
        // neither left nor right set – center horizontally
        pos.x = -dim.x * 0.5f;
    }

    // ----- vertical (y) -----
    if (margin.top != 0.0f) {
        // anchored to top edge (+1), move downward by margin.t and height
        pos.y =  1.0f - margin.top - dim.y;
    } else if (margin.bottom != 0.0f) {
        // anchored to bottom edge (‑1), move upward by margin.b
        pos.y = -1.0f + margin.bottom;
    } else {
        // neither top nor bottom set – center vertically
        pos.y = -dim.y * 0.5f;
    }

    return pos;
}


gui_t gui_init(style_t style)
{
    const ui_t root = {
        .is_dirty = true,
        .type = UI_PANEL,
        .data = {
            .state = {0},
            .label = str("ui panel"),
            .style = style
        },
        .left = NULL,
        .right = NULL
    };


    return (gui_t) {
        .root = mem_init(&root, sizeof(ui_t)),
        .gfx = {
            .vtx = list_init(quadf_t),
            .idx = list_init(u32),
            .shader = glshader_from_file_init(
                "lib/poglib/gui/uishader.vs", 
                "lib/poglib/gui/uishader.fs"
            )
        }
    };
}

void __pass_quad_to_renderer(ui_t *ui, gui_t *gui)
{
    const vec2f_t pos = __calculate_position_from_margin_and_width(ui->data.style);
    const quadf_t quad = quadf(
        (vec3f_t) { 
            pos.x, 
            pos.y + ui->data.style.dim.y, 
            -1.0f 
        },
        ui->data.style.dim.x,
        ui->data.style.dim.y
    );

    const u32 offset = gui->gfx.vtx.len;
    const u32 idx[] = {
        [0] = offset + DEFAULT_QUAD_INDICES[0],
        [1] = offset + DEFAULT_QUAD_INDICES[1],
        [2] = offset + DEFAULT_QUAD_INDICES[2],
        [3] = offset + DEFAULT_QUAD_INDICES[3],
        [4] = offset + DEFAULT_QUAD_INDICES[4],
        [5] = offset + DEFAULT_QUAD_INDICES[5],
    };


    list_append(&gui->gfx.vtx, quad);
    list_append_multiple(&gui->gfx.idx, idx);
}

void ui_render(ui_t *ui, gui_t *gui)
{
    if (!ui) return;

    /* TODO: Need to think on how to solve this
    if (!ui->is_dirty) {
        return;
    }
    */

    __pass_quad_to_renderer(ui, gui);
}

void gui_render(gui_t *self, const bool in_wireframe) 
{
    list_clear(&self->gfx.vtx);
    list_clear(&self->gfx.idx);

    ui_render(self->root, self);
    ui_render(self->root->left, self);
    ui_render(self->root->right, self);

    glrenderer3d_draw((glrendererconfig_t) {
        .calls = {
            .count = 1,
            .call = {
                [0] = {
                    .is_wireframe = in_wireframe,
                    .vtx = {
                        .data = self->gfx.vtx.data,
                        .size = list_get_size(&self->gfx.vtx)
                    },
                    .idx = {
                        .data = self->gfx.idx.data,
                        .nmemb = self->gfx.idx.len
                    },
                    .shader_config = {
                        .shader = &self->gfx.shader,
                        .uniforms = {0}
                    },
                    .attrs = {
                        .count = 1,
                        .attr = {
                            [0] = {
                                .ncmp = 3,
                                .interleaved = {0},
                            }
                        }
                    }
                }
            }
        }
    });
}

void ui_destroy(ui_t *self)
{
    if (self) {
        ui_destroy(self->left);
        ui_destroy(self->right);
        mem_free(self, sizeof(ui_t));
    }
}

void gui_destroy(gui_t *self)
{
    ASSERT(self);
    list_destroy(&self->gfx.vtx);
    glshader_destroy(&self->gfx.shader);
    list_destroy(&self->gfx.idx);
    ui_destroy(self->root);
}
