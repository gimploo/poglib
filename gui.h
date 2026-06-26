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
//4.Text truncate

//BUG:
//1. WDC coordinates text are not wrapping around to next line of NDC coordinates text while enclosed in the same composition

typedef enum {
    UI_BEHAVIOR_NONE                                 = 0,
    UI_BEHAVIOR_HOVERABLE                            = 1 << 0,
    UI_BEHAVIOR_CLICKABLE                            = 1 << 1,
    UI_BEHAVIOR_TRACK_STATE_TOGGLE                   = 1 << 2,
    UI_BEHAVIOR_TRACK_STATE_LOCK_MOUSE_ON_DRAG       = 1 << 3,
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

typedef enum {
    UI_LAYOUT_HORIZONTAL = 0,
    UI_LAYOUT_VERTICAL = 1
} ui_layout_t;

typedef struct {

    void *ref;
    u32 size;

} ui_valuebinding_t;

typedef struct {

    ui_layout_t layout;

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

    struct {
        vec4f_t base;
        vec4f_t highlight;
    } color;

    //WARN: AI introduced this, use with care - not tested!
    ui_text_align       text_align;
    ui_size_mode        size_mode;

    ui_valuebinding_t   binding;

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
    u32 max_col_width;
    ui_region_t starting_region; 
    ui_layout_t layout;
} cursor_ctx_t;

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
            cursor_ctx_t buffer[MAX_UI_NESTING_ALLOWED];
        } layout_cursor_stack;

        struct {
            u32 ui_id;
        } mouse_lock_on_ui;

        //NOTE: `is_mouse_on_ui` is to track whether mouse is on ui during the immediate mode rendering
        bool is_mouse_on_ui;

    } internal;

    struct {
        u32 hovered_ui_id;
    } state;

    ui_composition callback;
};


gui_t   gui_init(arena_t *const arena, const ui_region_t starting_region);

//INFO: define this in a function
void    gui_ui_compose_begin(gui_t * const gui, const ui_config_t config);
void    gui_ui_compose_end(gui_t *gui);

//INFO: pass above declared function that includes the gui compisition into here
void    gui_set_composition(gui_t * const self, ui_composition callback);

bool    gui_ui_ishovered(gui_t *const self, const u32 id);
bool    gui_ui_isclicked(gui_t *const self, const u32 id);

void    gui_run(gui_t *const self, const bool render_as_wireframe);

void    gui_destroy(gui_t *const self);



#ifndef IGNORE_GUI_IMPLEMENTATION

gui_t gui_init(arena_t *const arena, const ui_region_t starting_region)
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
    o.internal.layout_cursor_stack.buffer[0] = (cursor_ctx_t){
        .max_row_height = 0,
        .region = starting_region,
        .starting_region = starting_region,
        .max_col_width = 0
    };

    o.internal.mouse_lock_on_ui.ui_id = 0;

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

    if (!(config.composition.traits & UI_BEHAVIOR_HOVERABLE)) return;

    const ui_region_t region = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top]
        .region;

    const bool is_cursor_on_ui = ((f32)mouse_pos.x > region.cursor.x)
        && ((f32)mouse_pos.x < region.cursor.x + region.width)
        && ((f32)mouse_pos.y > region.cursor.y)
        && ((f32)mouse_pos.y < region.cursor.y + region.height);

    const bool is_ui_clicked = is_cursor_on_ui && window_mouse_button_just_pressed(global_window, SDL_MOUSEBUTTON_LEFT);

    gui->state.hovered_ui_id = is_cursor_on_ui
        ? config.id
        : gui->state.hovered_ui_id;

    gui->internal.is_mouse_on_ui = is_cursor_on_ui;

    if ((config.composition.traits & UI_BEHAVIOR_TRACK_STATE_TOGGLE) && is_ui_clicked) {
        ASSERT(config.binding.size == sizeof(bool));
        *(bool *)config.binding.ref = !*(bool *)config.binding.ref;
    }

    const bool is_released = window_mouse_button_is_released(global_window, SDL_MOUSEBUTTON_LEFT);
    if (config.composition.traits & UI_BEHAVIOR_TRACK_STATE_LOCK_MOUSE_ON_DRAG) {

        //TODO: the behavior here is to track the ui on mouse drag / click and on mouse click release
        //the ui is untracked

        if (is_ui_clicked && !gui->internal.mouse_lock_on_ui.ui_id) {
            gui->internal.mouse_lock_on_ui.ui_id = config.id;
        }

        if ((gui->internal.mouse_lock_on_ui.ui_id == config.id) && !is_released) {
            const vec2i_t rel = global_window->mouse.rel;
            *((f32 *)config.binding.ref) += (rel.x / 10.f);
        }

        if (is_released) {
            if (gui->internal.mouse_lock_on_ui.ui_id == config.id && !is_cursor_on_ui) {
                gui->internal.mouse_lock_on_ui.ui_id = 0;
            }
        }
    }

}

vec4f_t gui__internal_get_color(const gui_t *gui, const ui_config_t config)
{
    if ((config.composition.traits & UI_BEHAVIOR_HOVERABLE) == 0) {
        return config.color.base;
    }

    if ((config.composition.traits & UI_BEHAVIOR_CLICKABLE) == 0) {
        return config.color.base;
    }

    if ((config.composition.traits & UI_BEHAVIOR_TRACK_STATE_LOCK_MOUSE_ON_DRAG) && config.id == gui->internal.mouse_lock_on_ui.ui_id) {
        return config.color.highlight;
    }

    if (config.composition.traits & UI_BEHAVIOR_TRACK_STATE_TOGGLE) {
        ASSERT(config.binding.ref);
        return (*(bool *)config.binding.ref || gui->internal.is_mouse_on_ui) ? config.color.highlight : config.color.base;
    }
    return gui->internal.is_mouse_on_ui ? config.color.highlight : config.color.base;
}

void gui__internal_ui_push_cursor_layout(gui_t *gui, const cursor_ctx_t old_cursor__updated, const cursor_ctx_t new_cursor)
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

void gui_ui_compose_end(gui_t *gui)
{
    gui__internal_ui_pop_cursor_layout(gui);
}

void gui__internal_ui_validate_config(const ui_config_t config)
{
    ASSERT(config.dim.min_height);
    ASSERT(config.dim.min_width);

    if (config.composition.traits & UI_BEHAVIOR_CLICKABLE) {
        ASSERT(config.id != 0);
    }

    if (config.composition.traits & UI_BEHAVIOR_TRACK_STATE_TOGGLE) {
        ASSERT(config.binding.ref);
        ASSERT(config.binding.size == sizeof(bool));
        ASSERT((config.composition.traits & UI_BEHAVIOR_TRACK_STATE_LOCK_MOUSE_ON_DRAG) == 0);
    }

    if (config.composition.traits & UI_BEHAVIOR_TRACK_STATE_LOCK_MOUSE_ON_DRAG) {
        ASSERT(config.binding.ref);
        ASSERT(config.binding.size > sizeof(bool));
        ASSERT((config.composition.traits & UI_BEHAVIOR_TRACK_STATE_TOGGLE) == 0);
    }

    if (config.composition.styles & UI_STYLE_ONLY_TEXT) {
        ASSERT(config.text.len > 0);
    }
}

ui_region_t gui__internal_ui_add_child(gui_t *gui, const ui_config_t config)
{
    const cursor_ctx_t parent_layout = gui->internal
        .layout_cursor_stack
        .buffer[gui->internal.layout_cursor_stack.top];

    const ui_region_t parent_region         = parent_layout.region;
    vec2f_t parents_current_cursor          = parent_layout.region.cursor;
    const f32 child_height                  = (f32)config.dim.min_height;
    f32 child_width                         = (f32)config.dim.min_width;

    if (config.size_mode == UI_SIZE_FILL) {
        child_width = parent_region.width - (f32)(config.margin.left + config.margin.right);
    }

    const u32 child_region_width    = config.margin.left + child_width + config.margin.right;
    const u32 child_region_height   = config.margin.top + config.margin.bottom + child_height;


    //NOTE: handle cases where the generated current cursor is outside the containing region
    {
        const vec2f_t current_cursor = (vec2f_t ){
            .x = parents_current_cursor.x + config.margin.left,
            .y = parents_current_cursor.y + config.margin.top,
        };
        const bool is_next_cursor_outside_parent_region_horizontal  = (current_cursor.x  > (parent_layout.starting_region.cursor.x + parent_region.width));
        const bool is_next_cursor_outside_parent_region_vertical    = (current_cursor.y > (parent_layout.starting_region.cursor.y + parent_region.height));

        if (is_next_cursor_outside_parent_region_horizontal && parent_layout.layout == UI_LAYOUT_HORIZONTAL) {
            parents_current_cursor.x = parent_layout.starting_region.cursor.x;
            parents_current_cursor.y = parent_layout.starting_region.cursor.y + parent_layout.max_row_height;
        }

        if (is_next_cursor_outside_parent_region_vertical && parent_layout.layout == UI_LAYOUT_VERTICAL) {
            parents_current_cursor.x = parent_layout.starting_region.cursor.x + parent_layout.max_col_width;
            parents_current_cursor.y = parent_layout.starting_region.cursor.y;
        }
    }

    const ui_region_t child_region = {
        .cursor = (vec2f_t){
            .x = parents_current_cursor.x + config.margin.left,
            .y = parents_current_cursor.y + config.margin.top,
        },
        .width  = child_width,
        .height = child_height
    };

    const vec2f_t next_parent_cursor = parent_layout.layout == UI_LAYOUT_HORIZONTAL 
        ? (vec2f_t ){
            .x = parents_current_cursor.x + child_region_width,
            .y = parents_current_cursor.y
        } : (vec2f_t ){
            .x = parents_current_cursor.x,
            .y = parents_current_cursor.y + child_region_height
        };


    gui__internal_ui_push_cursor_layout(
        gui,
        (cursor_ctx_t) {
            .layout = parent_layout.layout,
            .region = (ui_region_t) {
                .cursor = next_parent_cursor,
                .width = parent_region.width,
                .height = parent_region.height
            },
            .max_row_height = MAX(parent_layout.max_row_height, child_region_height),
            .starting_region = parent_layout.starting_region,
            .max_col_width = MAX(parent_layout.max_col_width, child_region_width),
        },
        (cursor_ctx_t) {
            .layout = config.layout,
            .region = {
                .cursor = (vec2f_t){
                    .x = child_region.cursor.x + (f32)config.padding.left,
                    .y = child_region.cursor.y + (f32)config.padding.top
                },
                .height = child_region.height - (f32)(config.padding.top + config.padding.bottom),
                .width  = child_region.width  - (f32)(config.padding.left + config.padding.right),
            },
            .max_row_height     = child_region.height,
            .starting_region    = child_region,
            .max_col_width      = child_region.width
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

    if (config.text.len > 0) {
        ui_config_t text_cfg = config;
        text_cfg.color.base = (vec4f_t){1.0f, 1.0f, 1.0f, 1.0f};
        gui__internal_ui_create_text_internal(gui, child_region, text_cfg);
    }
}


void gui__internal_reset_layout_cursor(gui_t *self)
{
    const ui_region_t starting_region = self->internal.layout_cursor_stack.buffer[0].starting_region;
    self->internal.layout_cursor_stack.top = 0;
    self->internal.layout_cursor_stack.buffer[0] = (cursor_ctx_t){ 
        .region = starting_region,
        .max_row_height = 0,
        .max_col_width = 0,
        .starting_region = starting_region
    };
}

void gui__internal_reset_internals(gui_t *self)
{
    gui__internal_reset_layout_cursor(self);

    self->state.hovered_ui_id = 0;
    self->internal.is_mouse_on_ui = false;
}

void gui_run(gui_t *const self, const bool render_as_wireframe)
{
    if (!self->callback) {
        eprint("No ui composition provided!");
    }

    gui__internal_reset_internals(self);

    //NOTE: update starts here

    self->callback(self);


    //NOTE: render starts here
    glrenderer3d_draw((glrendererconfig_t) {
        .calls = {
            .count = 1,
            .call = {
               [0] = {
                    .is_wireframe = render_as_wireframe,
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
                                    .value.mat4 = glms_ortho(0.0f, global_window->width, global_window->height, 0.0f, -2.0f * MAX_UI_NESTING_ALLOWED, 2.0f * MAX_UI_NESTING_ALLOWED)
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
