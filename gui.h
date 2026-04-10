#pragma once
#include "poglib/application.h"
#include "poglib/application/window/sdl_window.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/color.h"
#include "poglib/basic/common.h"
#include "poglib/basic/ds/list.h"
#include "poglib/font/glfreetypefont.h"
#include "poglib/gfx/gl/common.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/gfx/gl/types.h"
#include "poglib/gfx/gl/vbo_stream_types.h"
#include "poglib/gfx/glrenderer3d.h"
#include <poglib/basic.h>
#include <poglib/math.h>

//INFO: This uses a stack based layout system -
//Cursor starts from top left with (vec2ui_t){0}
//Each cursor update is pushed to the stack

//TODO:
//1. Scroll
//2. Text wrapping 
//3. Rounded corners
//4. Way to have composition ui placed at the right end of the screen from the start
//5. Way for enclosed text to take up the entire parent dimension instead
//6. Toggle 
//7. Text truncate

//BUG:
//1. WDC coordinates text are not wrapping around to next line of NDC coordinates text while enclosed in the same composition

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
    ui_region_t    position;
    box_t          uv;
    vec4f_t        color;
    f32            corner_radius;
    f32            zorder;
    f32            is_text;
} ui_attr_t;

#define MAX_UI_NESTING_ALLOWED 4

typedef struct gui_t gui_t;

typedef void (*ui_composition)(const application_t * const app, gui_t *gui, ...);

typedef struct {
    ui_region_t region;
    u32 max_row_height;
    ui_region_t starting_region; 
} layout_ctx_t;

struct gui_t {
    arena_t arena;

    glfreetypefont_t freetypefont;
    glshader_t shader;

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

//INFO: define this in a function
void    gui_ui_compose_begin(gui_t * const gui, const ui_config_t config);
void    gui_ui_compose_end(gui_t *gui);

//INFO: pass above declared function that includes the gui compisition into here
void    gui_set_composition(gui_t * const self, ui_composition callback);

#define gui_update(self, app, ...) \
    ((self)->callback((app), (self), __VA_ARGS__))

void    gui_render(gui_t *self);
void    gui_destroy(gui_t *self);



#ifndef IGNORE_GUI_IMPLEMENTATION

gui_t gui_init(arena_t * const arena, const ui_region_t starting_region)
{
    return (gui_t){
        .shader =  glshader__file_init(
            str(POGLIB_ROOT_DIR"/gui/uishader-vtx.glsl"), 
            str(POGLIB_ROOT_DIR"/gui/uishader-frag.glsl"), 
            arena),
        .freetypefont = glfreetypefont_init(DEFAULT_FONT_ROBOTO_MEDIUM_FILEPATH, 14, true),
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
    glshader_destroy(&self->shader);
    list_destroy(&self->gfx.instanced_attrs);
}

ui_region_t gui__internal_get_current_region(gui_t *gui,const ui_config_t config);

vec4f_t gui__internal_get_color(gui_t *gui, ui_config_t config)
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

void gui__internal_ui_push_cursor_layout(gui_t *gui, const layout_ctx_t old_cursor__updated, const layout_ctx_t new_cursor)
{
    const u8 new_top = ++gui->internal.layout_cursor_stack.top;
    ASSERT(new_top < MAX_UI_NESTING_ALLOWED);
    gui->internal.layout_cursor_stack.buffer[new_top] = new_cursor; 
    gui->internal.layout_cursor_stack.buffer[new_top - 1] = old_cursor__updated; 
}

void gui__internal_ui_pop_cursor_layout(gui_t *gui)
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

void gui_ui_compose_end(gui_t *gui)
{
    gui__internal_ui_pop_cursor_layout(gui);
}

void gui__internal_ui_validate_config(ui_config_t config)
{
    ASSERT(config.dim.height > 0);
    ASSERT(config.dim.width > 0);
}

ui_region_t gui__internal_ui_add_child(gui_t *gui, const ui_config_t config)
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

    gui__internal_ui_push_cursor_layout(
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
            .region = {
                .cursor = (vec2f_t){ 
                    .x = child_region.cursor.x + config.padding.left,
                    .y = child_region.cursor.y + config.padding.top
                },
                .height = child_region.height,
                .width = child_region.width
            },
            .max_row_height = child_region.height,
            .starting_region = child_region
        }
    );
    return child_region;
}

void gui__internal_ui_create_text_internal(gui_t * const gui, const ui_region_t child_region, const ui_config_t config)
{
    vec2f_t starting_pos = child_region.cursor;
    const f32 offset = config.dim.width / config.label.len;
    for (u32 i = 0; i < config.label.len; i++)
    {
        const box_t quad = glfreetypefont_generate_uv_for_char(
                &gui->freetypefont, 
                config.label.data[i]);

        const u8 character = config.label.data[i];
        const f32 glyph_w = gui->freetypefont.fontatlas[character].bw;
        const f32 glyph_h = gui->freetypefont.fontatlas[character].bh;
        const f32 bearing_x = gui->freetypefont.fontatlas[character].bl;
        const f32 bearing_y = gui->freetypefont.fontatlas[character].bt;
        const f32 advance_x = gui->freetypefont.fontatlas[character].ax;

        const ui_attr_t attr = {
            .position = {
                .cursor = { 
                    floorf(starting_pos.x + bearing_x + bearing_x), 
                    floorf(starting_pos.y + (gui->freetypefont.fontsize - bearing_y))
                },
                .height = glyph_h,
                .width = glyph_w,
            },
            .color = config.color.base,
            .zorder = gui->internal.layout_cursor_stack.top,
            .uv = quad,
            .is_text = true,
        };

        list_append(&gui->gfx.instanced_attrs, attr);
        starting_pos.x += advance_x;
    }
}

void gui_ui_compose_begin(gui_t * const gui, const ui_config_t config)
{
    gui__internal_ui_validate_config(config);
    const ui_region_t child_region = gui__internal_ui_add_child(gui, config);
    const vec4f_t computed_color = gui__internal_get_color(gui, config);

    if (config.label.len) {
        gui__internal_ui_create_text_internal(gui, child_region, config);
        return;
    }

    const ui_attr_t attr = {
        .position = child_region,
        .color = computed_color,
        .zorder = gui->internal.layout_cursor_stack.top,
        .uv = {0},
        .is_text = false,
    };
    list_append(&gui->gfx.instanced_attrs, attr);
}

bool ui_is_clicked(const ui_t * const ui)
{
}



void gui__internal_ui_reset(gui_t *self)
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

    gui__internal_ui_reset(self);

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
                        .shader = &self->shader,
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
                    .textures = {
                        .count = 1,
                        .items = {
                            [0] = {
                                .type = GL_TEXTURE_TYPE_NORMAL,
                                .source = &self->freetypefont.texture
                            }
                        }
                    },
                    .attrs = {
                        .count = 6,
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
                                .type = GL_FLOAT,
                                .vbo_chunk_index = VBO_STREAM_TYPE_INSTANCE,
                                .interleaved = {
                                    .offset = offsetof(ui_attr_t, zorder),
                                    .stride = sizeof(ui_attr_t)
                                },
                            },
                            [4] = {
                                .ncmp = 4,
                                .type = GL_FLOAT,
                                .vbo_chunk_index = VBO_STREAM_TYPE_INSTANCE,
                                .interleaved = {
                                    .offset = offsetof(ui_attr_t, uv),
                                    .stride = sizeof(ui_attr_t)
                                },
                            },
                            [5] = {
                                .ncmp = 1,
                                .type = GL_FLOAT,
                                .vbo_chunk_index = VBO_STREAM_TYPE_INSTANCE,
                                .interleaved = {
                                    .offset = offsetof(ui_attr_t, is_text),
                                    .stride = sizeof(ui_attr_t)
                                }
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
