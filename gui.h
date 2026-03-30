#pragma once
#include "poglib/application/window/sdl_window.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/common.h"
#include "poglib/basic/ds/list.h"
#include "poglib/font/glfreetypefont.h"
#include "poglib/gfx/gl/shader.h"
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
    u32 x;
    u32 y;
} vec2ui_t;

typedef struct {
    vec2ui_t point[4];
} ui_quad_t;

typedef struct {
    vec2ui_t    position;
    vec2f_t     uv;
    vec4f_t     color;
    f32         corner_radius;
} ui_attr_t;

typedef ui_attr_t ui_glquad_t[4];

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

typedef struct {
    arena_t arena;

    glfreetypefont_t freetypefont;
    glshader_t shaders[UI_SHADER_COUNT];

    struct {
        list_t quads;
        list_t texts;
    } gfx;

    struct {
        struct {
            i8 top;
            vec2ui_t buffer[MAX_UI_NESTING_ALLOWED][UI_CURSOR_TYPE_COUNT];
        } layout_cursor_stack;
    } internal;
} gui_t;

gui_t   gui_init(arena_t * const arena);

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
            .quads = list_init(ui_glquad_t),
            .texts = list_init(ui_glquad_t)
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
}

ui_quad_t __ui_generate_quad(gui_t *gui,const ui_config_t config);

vec4f_t __ui_get_color(gui_t *gui, ui_config_t config)
{
    window_t *win = window_get_current_active_window();
    const vec2i_t mouse_pos = window_mouse_get_position(win);

    const ui_quad_t quad = __ui_generate_quad(gui, config);

    if (config.composition.traits & UI_BEHAVIOR_HOVERABLE) {

        const bool is_cursor_on_ui = 
            (f32)mouse_pos.x > quad.point[TOP_LEFT].x
            && (f32)mouse_pos.x < quad.point[TOP_RIGHT].x
            && (f32)mouse_pos.y > quad.point[TOP_LEFT].y
            && (f32)mouse_pos.y < quad.point[BOTTOM_LEFT].y;
        return config.color.highlight;
    }

    return config.color.base;
}


ui_quad_t __ui_generate_quad(gui_t *gui,const ui_config_t config)
{
    const vec2ui_t starting_point = gui->internal.layout_cursor_stack.buffer[gui->internal.layout_cursor_stack.top][UI_CURSOR_TYPE_INNER];

    return (ui_quad_t) {
        .point = {
            [TOP_LEFT] = {
                .x = starting_point.x + config.margin.left,
                .y = starting_point.y + config.margin.top
            },
            [TOP_RIGHT] = {
                .x = starting_point.x + config.margin.left + config.dim.width,
                .y = starting_point.y + config.margin.top,
            },
            [BOTTOM_LEFT] = {
                .x = starting_point.x + config.margin.left,
                .y = starting_point.y + config.margin.top + config.dim.height
            },
            [BOTTOM_RIGHT] = {
                .x = starting_point.x + config.margin.left + config.dim.width,
                .y = starting_point.y + config.margin.top + config.dim.height
            },
        } 
    };
}

void __ui_push_cursor_layout(gui_t *gui, const vec2ui_t outter_cursor, const vec2ui_t inner_cursor)
{
    const u8 new_top = ++gui->internal.layout_cursor_stack.top;
    gui->internal.layout_cursor_stack.buffer[new_top][UI_CURSOR_TYPE_OUTER] = outter_cursor; 
    gui->internal.layout_cursor_stack.buffer[new_top][UI_CURSOR_TYPE_INNER] = inner_cursor; 
}

void __ui_pop_cursor_layout(gui_t *gui)
{
    --gui->internal.layout_cursor_stack.top;
}

void ui_compose_end(gui_t *gui)
{
    __ui_pop_cursor_layout(gui);
}

vec2ui_t __ui_get_inner_cursor(gui_t *gui)
{
    return gui->internal.layout_cursor_stack.buffer[gui->internal.layout_cursor_stack.top][UI_CURSOR_TYPE_INNER];
}

void ui_compose_begin(gui_t *gui, const ui_config_t config)
{
    const vec2ui_t starting_point = __ui_get_inner_cursor(gui);
    const vec4f_t quad_color = __ui_get_color(gui, config);
    const ui_glquad_t quad = {
        [TOP_LEFT] = {
            .position = {
                .x = starting_point.x + config.margin.left,
                .y = starting_point.y + config.margin.top
            },
            .color = quad_color,
        },
        [TOP_RIGHT] = {
            .position = {
                .x = starting_point.x + config.margin.left + config.dim.width,
                .y = starting_point.y + config.margin.top,
            },
            .color = quad_color
        },
        [BOTTOM_LEFT] = {
            .position = {
                .x = starting_point.x + config.margin.left,
                .y = starting_point.y + config.margin.top + config.dim.height
            },
            .color = quad_color
        },
        [BOTTOM_RIGHT] = {
            .position = {
                .x = starting_point.x + config.margin.left + config.dim.width,
                .y = starting_point.y + config.margin.top + config.dim.height
            },
            .color = quad_color
        },
    };

    if (config.label.len) {
       eprint("Not implemented - use stb truetype");
    } else {
        list_append(&gui->gfx.quads, quad);
    }

    const vec2ui_t inner_position = {
        .x = quad[TOP_LEFT].position.x + config.padding.left,
        .y = quad[TOP_LEFT].position.y + config.padding.top
    };
    __ui_push_cursor_layout(gui, quad[TOP_RIGHT].position, inner_position);
}

bool ui_is_clicked(const ui_t * const ui)
{
}


void gui_render(gui_t *self, const matrix4f_t view)
{
    const matrix4f_t ortho_ndc = glms_ortho(0.0f, global_window->width, global_window->height, 0.0f, -2.0f, 2.0f);

    glrenderer3d_draw((glrendererconfig_t) {
        .calls = {
            .count = 2,
            .call = {
               [0] = {
                    .vtx = (buffer_t){
                        .raw_data = self->gfx.quads.data,
                        .size = list_get_size(&self->gfx.quads),
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
                        .count = 2,
                        .attr = {
                            [GL_VTX_ATTRIBUTE_TYPE_POSITION] = {
                                .ncmp = 2,
                                .type = GL_INT,
                                .interleaved = {
                                    .offset = offsetof(ui_attr_t, position),
                                    .stride = sizeof(ui_attr_t)
                                },
                            },
                            [GL_VTX_ATTRIBUTE_TYPE_COLOR] = {
                                .ncmp = 4,
                                .type = GL_FLOAT,
                                .interleaved = {
                                    .offset = offsetof(ui_attr_t, color),
                                    .stride = sizeof(ui_attr_t)
                                },
                            }
                        }
                    }
               },
               [1] = {
                   .allow_empty_vtx_buffer = true,
                   .vtx = (buffer_t){
                       .raw_data = self->gfx.texts.data,
                       .size = list_get_size(&self->gfx.texts),
                   },
                   .idx = {
                       .data = (u8 *)DEFAULT_QUAD_INDICES,
                       .nmemb = ARRAY_LEN(DEFAULT_QUAD_INDICES)
                   },
                   .instancing = {
                       .count = self->gfx.texts.len,
                       .enable = true
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
                           [GL_VTX_ATTRIBUTE_TYPE_POSITION] = {
                               .ncmp = 2,
                               .type = GL_INT,
                               .interleaved = {
                                   .offset = offsetof(ui_attr_t, position),
                                   .stride = sizeof(ui_attr_t)
                               },
                           },
                           [GL_VTX_ATTRIBUTE_TYPE_COLOR] = {
                               .ncmp = 4,
                               .type = GL_FLOAT,
                               .interleaved = {
                                   .offset = offsetof(ui_attr_t, color),
                                   .stride = sizeof(ui_attr_t)
                               },
                           },
                           [GL_VTX_ATTRIBUTE_TYPE_UV] = {
                               .ncmp = 2,
                               .type = GL_FLOAT,
                               .interleaved = {
                                   .offset = offsetof(ui_attr_t, uv),
                                   .stride = sizeof(ui_attr_t)
                               },
                           }
                       }
                   }
               }

            }
        }
    });

}
#endif
