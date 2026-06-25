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
//Cursor starts from top left with (vec2f_t){0}
//Each cursor update is pushed to the stack

//TODO:
//1.Scroll
//2.Text wrapping
//3.Way to have composition ui placed at the right end of the screen from the start
//4.Toggle
//5.Text truncate

//BUG:
//1. WDC coordinates text are not wrapping around to next line of NDC coordinates text while enclosed in the same composition

typedef enum {
    UI_BEHAVIOR_NONE        = 0,
    UI_BEHAVIOR_HOVERABLE   = 1 << 0,
    UI_BEHAVIOR_CLICKABLE   = 1 << 1,
} ui_trait_type;

typedef enum {
    UI_STYLE_NONE               = 0,
    UI_STYLE_ROUNDED_CORNERS    = 1 << 0,
    UI_STYLE_ONLY_TEXT          = 1 << 1,
} ui_style_config;

typedef enum {
    UI_TEXT_ALIGN_LEFT   = 0,
    UI_TEXT_ALIGN_CENTER = 1,
    UI_TEXT_ALIGN_RIGHT  = 2,
} ui_text_align;

typedef enum {
    UI_SIZE_WRAP = 0,
    UI_SIZE_FILL = 1,
} ui_size_mode;

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
        u32 top;
        u32 right;
        u32 bottom;
        u32 left;
    } padding, margin, border;

    u32 corner_radius;

    struct {
        u32 min_width;
        u32 min_height;
    } dim;

    u32 id;
    str_t text;
    u32 text_align;
    u32 size_mode;

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

#define MAX_UI_NESTING_ALLOWED 16

typedef struct gui_t gui_t;

typedef void (*ui_composition)(gui_t *gui, ...);

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

    struct {
        u32 hovered_ui_id;
    } state;

    ui_composition callback;
};


gui_t   gui_init(arena_t * const arena, const ui_region_t starting_region);

//INFO: define this in a function
void    gui_ui_compose_begin(gui_t * const gui, const ui_config_t config);
void    gui_ui_compose_end(gui_t *gui);

//INFO: pass above declared function that includes the gui compisition into here
void    gui_set_composition(gui_t * const self, ui_composition callback);

bool    gui_ui_ishovered(gui_t *const self, const u32 id);
bool    gui_ui_isclicked(gui_t *const self, const u32 id);

#define gui_update(SELF, ...) do {\
    gui__internal_reset_state(SELF);\
    ((SELF)->callback((SELF), __VA_ARGS__));\
} while(0);

void    gui_button(gui_t *gui, u32 id, str_t text, f32 w, f32 h, ui_config_t cfg);
void    gui_label(gui_t *gui, str_t text, f32 w, f32 h, ui_config_t cfg);
void    gui_render(gui_t *self);
void    gui_destroy(gui_t *self);



#ifndef IGNORE_GUI_IMPLEMENTATION

gui_t gui_init(arena_t * const arena, const ui_region_t starting_region)
{
    gui_t o = {0};
    o.shader =  glshader_init(
        str(POGLIB_ROOT_DIR"/gui/uishader-vtx.glsl"), 
        str(POGLIB_ROOT_DIR"/gui/uishader-frag.glsl"), 
        (gluniform_registry_t) {
            .count = 1,
            .data = {
                [0] = {
                    .name = str_lit("projection"),
                    .type = GL_UNIFORM_TYPE_MATRIX4F
                }
            }
        },
        arena
    );
    o.freetypefont = glfreetypefont_init(DEFAULT_FONT_ROBOTO_MEDIUM_FILEPATH, 14, true);
    o.gfx.instanced_attrs = list_init(ui_attr_t, arena);
    o.internal.layout_cursor_stack.top = 0;
    o.internal.layout_cursor_stack.buffer[0] = (layout_ctx_t){
        .max_row_height = 0,
        .region = starting_region,
        .starting_region = starting_region
    };
    return o;
}


void gui_destroy(gui_t *self)
{
    glshader_destroy(&self->shader);
    list_destroy(&self->gfx.instanced_attrs);
}

void gui__internal_update_state(gui_t *gui, const ui_config_t config)
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

    gui->state.hovered_ui_id = is_cursor_on_ui
        ? config.id
        : gui->state.hovered_ui_id;
}

bool gui__internal_is_cursor_on_ui(const gui_t *gui, const ui_config_t config)
{
    return gui->state.hovered_ui_id != 0 && config.id == gui->state.hovered_ui_id;
}

vec4f_t gui__internal_get_color(const gui_t *gui, const ui_config_t config)
{
    return gui__internal_is_cursor_on_ui(gui, config) ? config.color.highlight : config.color.base;
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

vec2f_t gui__internal_get_current_cursor(const gui_t *gui)
{
    const ui_region_t region = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top].region;

    return region.cursor;
}

void gui_ui_compose_end(gui_t *gui)
{
    gui__internal_ui_pop_cursor_layout(gui);
}

void gui__internal_ui_validate_config(const ui_config_t config)
{
    ASSERT(config.dim.min_height > 0);
    ASSERT(config.dim.min_width > 0);
    if (config.composition.traits & UI_BEHAVIOR_CLICKABLE) {
        ASSERT(config.id != 0);
    }

    if (config.composition.styles & UI_STYLE_ONLY_TEXT) {
        ASSERT(config.text.len > 0);
    }
}

ui_region_t gui__internal_ui_add_child(gui_t *gui, const ui_config_t config)
{
    const layout_ctx_t parent_layout = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top];

    const ui_region_t parent_region = parent_layout.region;
    const vec2f_t current_cursor = gui__internal_get_current_cursor(gui);

    f32 child_width  = (f32)config.dim.min_width;
    f32 child_height = (f32)config.dim.min_height;

    if (config.size_mode == UI_SIZE_FILL) {
        child_width = parent_region.width - (f32)(config.margin.left + config.margin.right);
    }

    const f32 width_enclosed_by_child_region = (f32)config.margin.left + child_width + (f32)config.margin.right;

    ui_region_t child_region = {
        .cursor.x = current_cursor.x + (f32)config.margin.left,
        .cursor.y = current_cursor.y + (f32)config.margin.top,
        .width  = child_width,
        .height = child_height
    };

    const f32 next_active_cursor_x = current_cursor.x + width_enclosed_by_child_region;

    const bool is_child_region_at_end_parent_layout_width = next_active_cursor_x >
        (parent_layout.starting_region.cursor.x + parent_region.width);

    if (is_child_region_at_end_parent_layout_width) {
        child_region.cursor = (vec2f_t){
            .x = parent_layout.starting_region.cursor.x + (f32)config.margin.left,
            .y = current_cursor.y + (f32)parent_layout.max_row_height + (f32)config.margin.top,
        };
    }

    const f32 row_height = (f32)config.margin.top + (f32)config.margin.bottom + child_height;

    gui__internal_ui_push_cursor_layout(
        gui,
        (layout_ctx_t) {
            .region = (ui_region_t) {
                .cursor = is_child_region_at_end_parent_layout_width
                    ? (vec2f_t){
                        parent_layout.starting_region.cursor.x + width_enclosed_by_child_region,
                        current_cursor.y + (f32)parent_layout.max_row_height
                    }
                    : (vec2f_t){
                        next_active_cursor_x,
                        current_cursor.y
                    },
                .width = parent_region.width,
                .height = parent_region.height
            },
            .max_row_height = MAX(parent_layout.max_row_height, (u32)row_height),
            .starting_region = parent_layout.starting_region
        },
        (layout_ctx_t) {
            .region = {
                .cursor = (vec2f_t){
                    .x = child_region.cursor.x + (f32)config.padding.left,
                    .y = child_region.cursor.y + (f32)config.padding.top
                },
                .height = child_region.height - (f32)(config.padding.top + config.padding.bottom) > 0.0f
                    ? child_region.height - (f32)(config.padding.top + config.padding.bottom)
                    : 0.0f,
                .width  = child_region.width  - (f32)(config.padding.left + config.padding.right) > 0.0f
                    ? child_region.width  - (f32)(config.padding.left + config.padding.right)
                    : 0.0f,
            },
            .max_row_height = (u32)child_region.height,
            .starting_region = child_region
        }
    );
    return child_region;
}

void gui__internal_ui_create_text_internal(gui_t * const gui, const ui_region_t child_region, const ui_config_t config)
{
    vec2f_t starting_pos = child_region.cursor;

    f32 total_text_width = 0.0f;
    for (u32 i = 0; i < config.text.len; i++)
        total_text_width += gui->freetypefont.fontatlas[(u8)config.text.data[i]].ax;

    if (config.text_align == UI_TEXT_ALIGN_CENTER)
        starting_pos.x = child_region.cursor.x + (child_region.width - total_text_width) / 2.0f;
    else if (config.text_align == UI_TEXT_ALIGN_RIGHT)
        starting_pos.x = child_region.cursor.x + child_region.width - total_text_width;

    for (u32 i = 0; i < config.text.len; i++)
    {
        const box_t quad = glfreetypefont_generate_uv_for_char(
                &gui->freetypefont,
                config.text.data[i]);

        const u8 character = config.text.data[i];
        const f32 glyph_w = gui->freetypefont.fontatlas[character].bw;
        const f32 glyph_h = gui->freetypefont.fontatlas[character].bh;
        const f32 bearing_x = gui->freetypefont.fontatlas[character].bl;
        const f32 bearing_y = gui->freetypefont.fontatlas[character].bt;
        const f32 advance_x = gui->freetypefont.fontatlas[character].ax;

        const ui_attr_t attr = {
            .position = {
                .cursor = {
                    floorf(starting_pos.x + bearing_x),
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

    gui__internal_update_state(gui, config);

    const vec4f_t computed_color = gui__internal_get_color(gui, config);

    if (config.composition.styles & UI_STYLE_ONLY_TEXT) {
        gui__internal_ui_create_text_internal(gui, child_region, config);
        return;
    }

    const ui_attr_t attr = {
        .position = child_region,
        .color = computed_color,
        .zorder = gui->internal.layout_cursor_stack.top,
        .uv = {0},
        .corner_radius = (config.composition.styles & UI_STYLE_ROUNDED_CORNERS)
            ? (f32)config.corner_radius
            : 0.0f,
        .is_text = false,
    };
    list_append(&gui->gfx.instanced_attrs, attr);

    if (config.text.len > 0)
        gui__internal_ui_create_text_internal(gui, child_region, config);
}

void gui_button(gui_t *gui, u32 id, str_t text, f32 w, f32 h, ui_config_t cfg)
{
    cfg.id = id;
    cfg.composition.traits = UI_BEHAVIOR_HOVERABLE | UI_BEHAVIOR_CLICKABLE;
    cfg.composition.styles = UI_STYLE_ROUNDED_CORNERS;
    cfg.dim.min_width  = (u32)w;
    cfg.dim.min_height = (u32)h;
    cfg.text = text;
    gui_ui_compose_begin(gui, cfg);
    gui_ui_compose_end(gui);
}

void gui_label(gui_t *gui, str_t text, f32 w, f32 h, ui_config_t cfg)
{
    cfg.composition.styles = UI_STYLE_ONLY_TEXT;
    cfg.dim.min_width  = (u32)w;
    cfg.dim.min_height = (u32)h;
    cfg.text = text;
    gui_ui_compose_begin(gui, cfg);
    gui_ui_compose_end(gui);
}



void gui__internal_reset_layout_cursor(gui_t *self)
{
    const ui_region_t starting_region = self->internal.layout_cursor_stack.buffer[0].starting_region;
    self->internal.layout_cursor_stack.top = 0;
    self->internal.layout_cursor_stack.buffer[0] = (layout_ctx_t){ 
        .region = starting_region,
        .max_row_height = 0,
        .starting_region = starting_region
    };
}

void gui__internal_reset_state(gui_t *self)
{
    self->state.hovered_ui_id = 0;
}

void gui_render(gui_t *self)
{
    if (!self->callback) {
        eprint("No ui composition provided!");
    }

    gui__internal_reset_layout_cursor(self);

    const matrix4f_t ortho_ndc = glms_ortho(0.0f, global_window->width, global_window->height, 0.0f, -2.0f * MAX_UI_NESTING_ALLOWED, 2.0f * MAX_UI_NESTING_ALLOWED);

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
                            .data = {
                                [0] = {
                                    .name = str_lit("projection"),
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
                        .count = 7,
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
                            },
                            [6] = {
                                .ncmp = 1,
                                .type = GL_FLOAT,
                                .vbo_chunk_index = VBO_STREAM_TYPE_INSTANCE,
                                .interleaved = {
                                    .offset = offsetof(ui_attr_t, corner_radius),
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

bool gui_ui_ishovered(gui_t *const self, const u32 id)
{
    return self->state.hovered_ui_id != 0 && self->state.hovered_ui_id == id;
}

bool gui_ui_isclicked(gui_t *const self, const u32 id)
{
    ASSERT(global_window);
    return gui_ui_ishovered(self, id) && window_mouse_button_is_pressed(global_window, SDL_MOUSEBUTTON_LEFT);
}



#endif
