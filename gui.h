#pragma once
#include "poglib/application.h"
#include "poglib/application/window/sdl_window.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/common.h"
#include "poglib/basic/ds/list.h"
#include "poglib/font/glfreetypefont.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/gfx/gl/types.h"
#include "poglib/gfx/gl/vbo_stream_types.h"
#include "poglib/gfx/glrenderer3d.h"
#include "poglib/math/shapes.h"
#include <poglib/basic.h>
#include <poglib/math.h>

//TODO:
//1. Scroll

typedef enum {
    UI_BEHAVIOR_NONE        = 0 << 1,
    UI_BEHAVIOR_HOVERABLE   = 1 << 1,
    UI_BEHAVIOR_CLICKABLE   = 2 << 1,
} ui_trait_type;

typedef enum {
    UI_STYLE_NONE               = 0 << 1,
    UI_STYLE_ROUNDED_CORNERS    = 1 << 1
} ui_style_config;

typedef struct {
    vec2f_t cursor;
    f32 width;
    f32 height;
}  ui_region_t;

typedef struct {

    struct {
        u32 traits; 
        u32 styles; 
    } composition;

    struct {
        u32 left;
        u32 right;
        u32 bottom;
        u32 top;
    } padding, margin, border, corners;

    struct {
        u32 width;
        u32 height;
    } dim;

    str_t label;

    struct {
        vec4f_t base;
        vec4f_t highlight;
    } color;

} ui_config_t;

typedef ui_config_t ui_t;

typedef struct {
    ui_region_t     position;
    ui_region_t     uv;
    vec4f_t         color;
    f32             corner_radius;
    u32             zorder;
} ui_attr_t;

typedef enum {
    UI_SHADER_DEFAULT = 0,
    UI_SHADER_FONT = 1,
    UI_SHADER_COUNT
} ui_shader_type;

#define MAX_UI_NESTING_ALLOWED 5

typedef struct gui_t gui_t;

typedef void (*ui_composition)(const application_t * const app, gui_t *gui);

typedef struct {
    ui_region_t region;
    u32 max_row_height;
    ui_region_t starting_region; 
} layout_ctx_t;

struct gui_t {
    arena_t arena;

    glfreetypefont_t freetypefont;
    glshader_t shaders[UI_SHADER_COUNT];

    struct {
        list_t instanced_attrs;
    } gfx;

    struct {
        struct {
            i8 top;
            layout_ctx_t buffer[MAX_UI_NESTING_ALLOWED];
        } layout_cursor_stack;
    } internal;

    ui_composition callback;
};


gui_t   gui_init(arena_t * const arena, const ui_region_t starting_region);
void    gui_set_composition(gui_t * const self, ui_composition callback);
void    gui_render(gui_t *self);
void    gui_destroy(gui_t *self);


#ifndef IGNORE_GUI_IMPLEMENTATION

gui_t gui_init(arena_t * const arena, const ui_region_t starting_region)
{
    return (gui_t){
        .shaders = {
            [UI_SHADER_DEFAULT] = glshader__file_init(
                    str(POGLIB_ROOT_DIR"/gui/uishader-vtx.glsl"), 
                    str(POGLIB_ROOT_DIR"/gui/uishader-frag.glsl"), 
                    arena),
            [UI_SHADER_FONT] = glshader__file_init(
                    str(POGLIB_ROOT_DIR  "/gui/ui-text-shader.vs"),
                    str(POGLIB_ROOT_DIR  "/gui/ui-text-shader.fs"),
                    arena),
        },
        .arena = arena_init(arena, 1 * MB),
        .gfx = {
            .instanced_attrs = list_init(ui_attr_t),
        },
        .internal = {
            .layout_cursor_stack = {
                .top = 0,
                .buffer = {
                    [0] = (layout_ctx_t){
                        .max_row_height = 0,
                        .region = starting_region,
                        .starting_region = starting_region
                    }
                },
            },
        }
    };
}


void gui_destroy(gui_t *self)
{
    glshader_destroy(&self->shaders[UI_SHADER_DEFAULT]);
    glshader_destroy(&self->shaders[UI_SHADER_FONT]);
    list_destroy(&self->gfx.instanced_attrs);
}

ui_region_t __get_current_region(gui_t *gui,const ui_config_t config);

vec4f_t __ui_get_color(gui_t *gui, ui_config_t config)
{
    window_t *win = window_get_current_active_window();
    const vec2i_t mouse_pos = window_mouse_get_position(win);

    const ui_region_t region = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top]
        .region;

    const bool is_cursor_on_ui = (f32)mouse_pos.x > region.cursor.x
        && (f32)mouse_pos.x < region.cursor.x + region.width
        && (f32)mouse_pos.y > region.cursor.y
        && (f32)mouse_pos.y < region.cursor.y + region.height;

    if ((config.composition.traits & UI_BEHAVIOR_HOVERABLE) && is_cursor_on_ui) {
        return config.color.highlight;
    }

    return config.color.base;
}

void __ui_push_cursor_layout(gui_t *gui, const layout_ctx_t old_parent_layout, const layout_ctx_t new_parent_layout)
{
    const u8 new_top = ++gui->internal.layout_cursor_stack.top;
    ASSERT(new_top < MAX_UI_NESTING_ALLOWED);
    gui->internal.layout_cursor_stack.buffer[new_top] = new_parent_layout; 
    gui->internal.layout_cursor_stack.buffer[new_top - 1] = old_parent_layout; 
}

void __ui_pop_cursor_layout(gui_t *gui)
{
    ASSERT(gui->internal.layout_cursor_stack.top > -1);
    --gui->internal.layout_cursor_stack.top;
}

vec2f_t __get_active_cursor(gui_t *gui)
{
    return gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top].region.cursor;
}

void ui_compose_end(gui_t *gui)
{
    __ui_pop_cursor_layout(gui);
}

void __validate_ui_config(ui_config_t config)
{
    ASSERT(config.dim.height > 0);
    ASSERT(config.dim.width > 0);
}

ui_region_t __ui_add_child(gui_t *gui, const ui_config_t config )
{
    const layout_ctx_t parent_layout = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top];

    const ui_region_t parent_region = parent_layout.region;

    const vec2f_t current_cursor = __get_active_cursor(gui);

    ui_region_t child_region = {
        .cursor.x = current_cursor.x + config.margin.left,
        .cursor.y = current_cursor.y + config.margin.top,
        .width  = config.dim.width,
        .height = config.dim.height
    };

    const u32 width_enclosed_by_child_region = config.margin.left + config.dim.width + config.margin.right;
    const u32 next_active_cursor_x = current_cursor.x + width_enclosed_by_child_region;

    const bool is_child_region_at_end_parent_layout_width = next_active_cursor_x > 
        ((u32)parent_layout.starting_region.cursor.x + parent_region.width);

    if (is_child_region_at_end_parent_layout_width) {
        child_region.cursor = (vec2f_t){
            .x = parent_layout.starting_region.cursor.x + config.margin.left,
            .y = current_cursor.y + parent_layout.max_row_height + config.margin.top,
        };
    }

    const u32 row_height_inclosed_by_child_region = config.margin.top + config.margin.bottom + config.dim.height;

    __ui_push_cursor_layout(
        gui, 
        (layout_ctx_t) {
            .region = (ui_region_t) {
                .cursor = is_child_region_at_end_parent_layout_width 
                    ? (vec2f_t){
                        parent_layout.starting_region.cursor.x + width_enclosed_by_child_region,
                        current_cursor.y + parent_layout.max_row_height 
                    }
                    : (vec2f_t){
                        next_active_cursor_x,
                        current_cursor.y 
                    },
                .width = parent_region.width,
                .height = parent_region.height
            },
            .max_row_height = !is_child_region_at_end_parent_layout_width 
                ? MAX(parent_layout.max_row_height, row_height_inclosed_by_child_region)
                : 0,
            .starting_region = parent_layout.starting_region
        },
        (layout_ctx_t) {
            .region = child_region,
            .max_row_height = child_region.height,
            .starting_region = child_region
        }
    );
    return child_region;
}

void ui_compose_begin(gui_t *gui, const ui_config_t config)
{
    __validate_ui_config(config);

    const ui_region_t child_region = __ui_add_child(gui, config);

    const vec4f_t quad_color = __ui_get_color(gui, config);

    if (config.label.len) {
       eprint("Not implemented - use stb truetype");
    } else {
        const ui_attr_t attr = {
            .position = child_region,
            .color = quad_color,
            .zorder = gui->internal.layout_cursor_stack.top
        };
        list_append(&gui->gfx.instanced_attrs, attr);
    }
}

bool ui_is_clicked(const ui_t * const ui)
{
}


void gui_update(gui_t *self, const application_t * const app)
{
    self->callback(app, self);
}

void __reset_gui_internals(gui_t *self)
{
    const ui_region_t starting_region = self->internal.layout_cursor_stack.buffer[0].starting_region;
    self->internal.layout_cursor_stack.top = 0;
    self->internal.layout_cursor_stack.buffer[0] = (layout_ctx_t){ 
        .region = starting_region,
        .max_row_height = 0,
        .starting_region = starting_region
    };
}

void gui_render(gui_t *self)
{
    if (!self->callback) {
        eprint("No ui composition provided!");
    }

    __reset_gui_internals(self);

    const matrix4f_t ortho_ndc = glms_ortho(0.0f, global_window->width, global_window->height, 0.0f, -2.0f, 2.0f);

    glrenderer3d_draw((glrendererconfig_t) {
        .calls = {
            .count = 1,
            .call = {
               [0] = {
                    .disable_depth_buffer = true,
                    .instancing = {
                        .enable = true,
                        .count =  self->gfx.instanced_attrs.len
                    },
                    .vtx = {
                        [VBO_STREAM_TYPE_GEOMETRY] = {
                            .raw_data = (u8 *)DEFAULT_QUAD__4TH_QUADRANT_VTX,
                            .size = sizeof(DEFAULT_QUAD__4TH_QUADRANT_VTX)
                        },
                        [VBO_STREAM_TYPE_INSTANCE] = {
                            .raw_data = self->gfx.instanced_attrs.data,
                            .size = list_get_size(&self->gfx.instanced_attrs),
                        },
                    },
                    .idx = {
                        .data = (u8 *)DEFAULT_QUAD_INDICES,
                        .nmemb = ARRAY_LEN(DEFAULT_QUAD_INDICES)
                    },
                    .shader_config = {
                        .shader = &self->shaders[UI_SHADER_DEFAULT],
                        .uniforms = {
                            .count = 1,
                            .uniform = {
                                [0] = {
                                    .name = "projection",
                                    .type = "matrix4f_t",
                                    .value.mat4 = ortho_ndc
                                },
                            }
                        }
                    },
                    .attrs = {
                        .count = 4,
                        .attr = {
                            [0] = {
                                .ncmp = 2,
                                .type = GL_FLOAT,
                                .vbo_chunk_index = VBO_STREAM_TYPE_GEOMETRY,
                                .interleaved = {0},
                            },
                            [1] = {
                                .ncmp = 4,
                                .type = GL_FLOAT,
                                .vbo_chunk_index = VBO_STREAM_TYPE_INSTANCE,
                                .interleaved = {
                                    .offset = offsetof(ui_attr_t, position),
                                    .stride = sizeof(ui_attr_t)
                                },
                            },
                            [2] = {
                                .ncmp = 4,
                                .type = GL_FLOAT,
                                .vbo_chunk_index = VBO_STREAM_TYPE_INSTANCE,
                                .interleaved = {
                                    .offset = offsetof(ui_attr_t, color),
                                    .stride = sizeof(ui_attr_t)
                                },
                            },
                            [3] = {
                                .ncmp = 1,
                                .type = GL_INT,
                                .vbo_chunk_index = VBO_STREAM_TYPE_INSTANCE,
                                .interleaved = {
                                    .offset = offsetof(ui_attr_t, zorder),
                                    .stride = sizeof(ui_attr_t)
                                },
                            }
                        }
                    }
               },
            }
        }
    });

    list_clear(&self->gfx.instanced_attrs);
}

void gui_set_composition(gui_t * const self, ui_composition callback)
{
    self->callback = callback;
}
#endif
