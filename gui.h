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
    f32 x;
    f32 y;
    f32 max_width;
    f32 max_height;
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
    ui_region_t    position;
    ui_region_t    uv;
    vec4f_t     color;
    f32         corner_radius;
} ui_attr_t;

typedef enum {
    UI_SHADER_DEFAULT = 0,
    UI_SHADER_FONT = 1,
    UI_SHADER_COUNT
} ui_shader_type;

#define MAX_UI_NESTING_ALLOWED 5

typedef enum {
    UI_LAYOUT_CURSOR_CHILD = 0,
    UI_LAYOUT_CURSOR_PARENT = 1,
    UI_LAYOUT_CURSOR_COUNT
} ui_layout_cursor_type;

typedef struct gui_t gui_t;

typedef void (*ui_composition)(const application_t * const app, gui_t *gui);

typedef struct {
    ui_region_t region;
    u32 max_row_height;
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
            layout_ctx_t buffer[MAX_UI_NESTING_ALLOWED][UI_LAYOUT_CURSOR_COUNT];
        } layout_cursor_stack;
        ui_region_t starting_region;

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
                    str(POGLIB_ROOT_DIR"/gui/uishader.vs"), 
                    str(POGLIB_ROOT_DIR"/gui/uishader.fs"), 
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
                //NOTE: start of with the entire window as the region
                .top = 0,
                .buffer = {
                    [UI_LAYOUT_CURSOR_PARENT] = (layout_ctx_t){ 
                        .region = starting_region,
                        .max_row_height = 0
                    },
                    [UI_LAYOUT_CURSOR_CHILD] = (layout_ctx_t){ 0 }
                }
            },
            .starting_region = starting_region
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
        .buffer[gui->internal.layout_cursor_stack.top][UI_LAYOUT_CURSOR_CHILD]
        .region;

    const bool is_cursor_on_ui = (f32)mouse_pos.x > region.x
        && (f32)mouse_pos.x < region.x + region.max_width
        && (f32)mouse_pos.y > region.y
        && (f32)mouse_pos.y < region.y + region.max_height;

    if ((config.composition.traits & UI_BEHAVIOR_HOVERABLE) && is_cursor_on_ui) {
        return config.color.highlight;
    }

    return config.color.base;
}

void __ui_push_cursor_layout(gui_t *gui, const layout_ctx_t old_parent_layout, const layout_ctx_t new_parent_layout)
{
    const u8 new_top = ++gui->internal.layout_cursor_stack.top;
    ASSERT(new_top < MAX_UI_NESTING_ALLOWED);
    gui->internal.layout_cursor_stack.buffer[new_top][UI_LAYOUT_CURSOR_PARENT] = old_parent_layout; 
    gui->internal.layout_cursor_stack.buffer[new_top][UI_LAYOUT_CURSOR_CHILD] = new_parent_layout; 
}

void __ui_pop_cursor_layout(gui_t *gui)
{
    ASSERT(gui->internal.layout_cursor_stack.top > -1);
    --gui->internal.layout_cursor_stack.top;
}

void __ui_update_row_height_of_parent(gui_t *gui)
{
    const u32 latest_row_height = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top + 1][UI_LAYOUT_CURSOR_PARENT].max_row_height;

    const u32 old_row_height = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top][UI_LAYOUT_CURSOR_CHILD].max_row_height; 

    gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top][UI_LAYOUT_CURSOR_CHILD].max_row_height = MAX(latest_row_height, old_row_height);
}

void __ui_update_parent_cursor(gui_t *gui)
{
    const u32 x_offset = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top + 1][UI_LAYOUT_CURSOR_PARENT].region.x;

    const u32 y_offset = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top + 1][UI_LAYOUT_CURSOR_PARENT].region.max_height;

    gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top][UI_LAYOUT_CURSOR_CHILD].region.x = x_offset;
}

void ui_compose_end(gui_t *gui)
{
    __ui_pop_cursor_layout(gui);
    __ui_update_row_height_of_parent(gui);
    __ui_update_parent_cursor(gui);
}

void ui_compose_begin(gui_t *gui, const ui_config_t config)
{
    const layout_ctx_t parent_layout = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top][UI_LAYOUT_CURSOR_CHILD];

    const ui_region_t parent_region = parent_layout.region;

    const ui_region_t child_region = {
        .x      = parent_region.x + config.margin.left,
        .y      = parent_region.y + config.margin.top,
        .max_width  = config.dim.width,
        .max_height = config.dim.height
    };

    f32 total_step_x = config.margin.left + config.dim.width + config.margin.right;

    __ui_push_cursor_layout(
        gui, 
        (layout_ctx_t) {
            .region = (ui_region_t) {
                .x = parent_region.x + total_step_x,
                .y = parent_region.y,
                .max_width = parent_region.max_width,
                .max_height = parent_region.max_height
            },
            .max_row_height = MAX(parent_layout.max_row_height, child_region.max_height)
        },
        (layout_ctx_t) {
            .region = child_region,
            .max_row_height = child_region.max_height
        }
    );

    const vec4f_t quad_color = __ui_get_color(gui, config);

    if (config.label.len) {
       eprint("Not implemented - use stb truetype");
    } else {
        const ui_attr_t attr = {
            .position = child_region,
            .color = quad_color,
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
    self->internal.layout_cursor_stack.top = 0;
    self->internal.layout_cursor_stack.buffer[0][UI_LAYOUT_CURSOR_PARENT] = (layout_ctx_t){ 
        .region = self->internal.starting_region,
        .max_row_height = 0
    };
    self->internal.layout_cursor_stack.buffer[0][UI_LAYOUT_CURSOR_CHILD] = (layout_ctx_t){0};
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
                        .count = 3,
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
