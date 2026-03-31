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
    UI_BEHAVIOR_HOVERABLE = 0 << 1,
    UI_BEHAVIOR_CLICKABLE = 1 << 1,
} ui_trait_type;

typedef enum {
    UI_STYLE_ROUNDED_CORNERS = 0 << 1
} ui_style_config;

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
    region_t    position;
    region_t    uv;
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
    UI_CURSOR_TYPE_INNER = 0,
    UI_CURSOR_TYPE_OUTER = 1,
    UI_CURSOR_TYPE_COUNT
} ui_layout_cursor_type;

typedef struct gui_t gui_t;

typedef void (*ui_composition)(const application_t * const app, gui_t *gui);

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
            region_t buffer[MAX_UI_NESTING_ALLOWED][UI_CURSOR_TYPE_COUNT];
        } layout_cursor_stack;
    } internal;

    ui_composition callback;
};


gui_t   gui_init(arena_t * const arena);
void    gui_set_composition(gui_t * const self, ui_composition callback);
void    gui_render(gui_t *self, const matrix4f_t view);
void    gui_destroy(gui_t *self);


#ifndef IGNORE_GUI_IMPLEMENTATION

gui_t gui_init(arena_t * const arena)
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
                .top = 0,
                .buffer = {0}
            }
        }
    };
}


void gui_destroy(gui_t *self)
{
    glshader_destroy(&self->shaders[UI_SHADER_DEFAULT]);
    glshader_destroy(&self->shaders[UI_SHADER_FONT]);
    list_destroy(&self->gfx.instanced_attrs);
}

region_t __get_current_region(gui_t *gui,const ui_config_t config);

vec4f_t __ui_get_color(gui_t *gui, ui_config_t config)
{
    window_t *win = window_get_current_active_window();
    const vec2i_t mouse_pos = window_mouse_get_position(win);

    const region_t region = __get_current_region(gui, config);

    if (config.composition.traits & UI_BEHAVIOR_HOVERABLE) {

        const bool is_cursor_on_ui = (f32)mouse_pos.x > region.x
            && (f32)mouse_pos.x < region.x + region.width
            && (f32)mouse_pos.y > region.y
            && (f32)mouse_pos.y < region.y + region.height;
        return config.color.highlight;
    }

    return config.color.base;
}

region_t __get_current_region(gui_t *gui,const ui_config_t config)
{
    return gui->internal.layout_cursor_stack.buffer[gui->internal.layout_cursor_stack.top][UI_CURSOR_TYPE_INNER];
}

void __ui_push_cursor_region(gui_t *gui, const region_t outter_cursor_region, const region_t inner_cursor_region)
{
    const u8 new_top = ++gui->internal.layout_cursor_stack.top;
    gui->internal.layout_cursor_stack.buffer[new_top][UI_CURSOR_TYPE_OUTER] = outter_cursor_region; 
    gui->internal.layout_cursor_stack.buffer[new_top][UI_CURSOR_TYPE_INNER] = inner_cursor_region; 
}

void __ui_pop_cursor_layout(gui_t *gui)
{
    --gui->internal.layout_cursor_stack.top;
}

void ui_compose_end(gui_t *gui)
{
    __ui_pop_cursor_layout(gui);
}

region_t __ui_get_inner_cursor_region(gui_t *gui)
{
    return gui->internal.layout_cursor_stack.buffer[gui->internal.layout_cursor_stack.top][UI_CURSOR_TYPE_INNER];
}

void ui_compose_begin(gui_t *gui, const ui_config_t config)
{
    const region_t current_region = __ui_get_inner_cursor_region(gui);
    const region_t inner_region = {
        .x = current_region.x + config.margin.left,
        .y = current_region.y + config.margin.top,
        .width = config.dim.width,
        .height = config.dim.height
    };
    const vec4f_t quad_color = __ui_get_color(gui, config);

    if (config.label.len) {
       eprint("Not implemented - use stb truetype");
    } else {
        const ui_attr_t attr = {
            .position = inner_region,
            .color = quad_color,
        };
        list_append(&gui->gfx.instanced_attrs, attr);
    }
    __ui_push_cursor_region(gui, current_region, inner_region);
}

bool ui_is_clicked(const ui_t * const ui)
{
}


void gui_update(gui_t *self, const application_t * const app)
{
    self->callback(app, self);
}

void gui_render(gui_t *self, const matrix4f_t view)
{
    if (!self->callback) {
        eprint("No ui composition provided!");
    }

    const matrix4f_t ortho_ndc = glms_ortho(0.0f, global_window->width, global_window->height, 0.0f, -2.0f, 2.0f);

    glrenderer3d_draw((glrendererconfig_t) {
        .calls = {
            .count = 1,
            .call = {
               [0] = {
                    .instancing = {
                        .enable = true,
                        .count =  self->gfx.instanced_attrs.len
                    },
                    .vtx = {
                        [VBO_STREAM_TYPE_GEOMETRY] = {
                            .raw_data = (u8 *)DEFAULT_QUAD_VTX,
                            .size = sizeof(DEFAULT_QUAD_VTX)
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
                                [1] = {
                                    .name = "view",
                                    .type = "matrix4f_t",
                                    .value.mat4 = view
                                }
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
