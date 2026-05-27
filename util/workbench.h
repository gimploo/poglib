#pragma once
#include <poglib/poggen.h>
#include <poglib/gui.h>
#include "./glcamera.h"
#include <poglib/gfx/glrenderer2d.h>
#include <poglib/gfx/glrenderer3d.h>
#include "./workbench/workbench-constants.h"
#include "./gllight.h"
#include "./workbench/ui/workbench-ui.h"
#include "./workbench/workbench-grid.h"
#include "poglib/application.h"
#include "poglib/application/window/sdl_window.h"
#include "poglib/basic/common.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/util/assetmanager.h"
#include "poglib/util/spriteatlas.h"

typedef struct {

    vec3f_t start;
    vec3f_t end;

} line_t;

typedef struct {

    struct {
        bool wireframe_mode;
    } render_config;

    struct {
        gui_t handle;
        bool enable;
    } gui;

    struct {
        spriteatlas_t atlas;
        glshader_t shader;
    } primitives;

    glshader_t shader;
    glcamera_t world_camera;
    vec3f_t player_camera_position;

    // Lines that draws - this clears up after every render
    list_t draw_lines;

    list_t lightsources;

} workbench_t;

workbench_t workbench_init(arena_t * const arena);

void        workbench_update_player_camera_position(workbench_t *self, const vec3f_t pos);
void        workbench_pass_line(workbench_t *self, const line_t line);
void        workbench_track_lightsource(workbench_t *self, const gllight_t *light);
void        workbench_toggle_wireframe_mode(workbench_t *self);
void        workbench_toggle_gui(workbench_t *self);
void        workbench_update_world_camera(workbench_t * const self, const f32 dt);

void        workbench_render_push_cube(workbench_t * const self, const vec3f_t translation, const vec3f_t scale, const vec4f_t color, const bool override_wireframe);
void        workbench_render(workbench_t *self);

void        workbench_destroy(workbench_t *self);


#define WORKBENCH_CAMERA_DEFAULT_POSITION (vec3f_t){0.f, 0.f, 10.f}
#define WORKBENCH_CAMERA_DEFAULT_ROTATION (vec2f_t){0}

workbench_t workbench_init(arena_t * const arena)
{
    ASSERT(global_poggen);
    workbench_t o = {
        .shader = glshader_init(
            str(POGLIB_ROOT_DIR"/util/workbench/workbench-shader.vs"), 
            str(POGLIB_ROOT_DIR"/util/workbench/workbench-shader.fs"),
            (gluniform_registry_t){
                .count = 5,
                .data = {
                    [0] = {
                        .name = str_lit("view"),
                        .type = GL_UNIFORM_TYPE_MATRIX4F
                    },
                    [1] = {
                        .name = str_lit("projection"),
                        .type = GL_UNIFORM_TYPE_MATRIX4F
                    },
                    [2] = {
                        .name = str_lit("transform"),
                        .type = GL_UNIFORM_TYPE_MATRIX4F
                    },
                    [3] = {
                        .name = str_lit("color"),
                        .type = GL_UNIFORM_TYPE_VEC4F
                    },
                    [4] = {
                        .name = str_lit("cameraPos"),
                        .type = GL_UNIFORM_TYPE_VEC3F
                    }
                }
            },
            arena
        ),
        .primitives = {
            .shader  = glshader_init(
                str(POGLIB_ROOT_DIR"/pipeline/render/shader/instance-vtx.glsl"),
                str(POGLIB_ROOT_DIR"/pipeline/render/shader/instance-frag.glsl"),
                (gluniform_registry_t){ 
                    .count = 2,
                    .data = {
                        [0] = {
                            .name = str_lit("projection"),
                            .type = GL_UNIFORM_TYPE_MATRIX4F
                        },
                        [1] = {
                            .name = str_lit("view"),
                            .type = GL_UNIFORM_TYPE_MATRIX4F
                        },
                    }
                },
                arena
            ),
            .atlas = spriteatlas_init(str(POGLIB_ROOT_DIR"/res/sprites/prototype.png"), 8, 4, arena),
        },
        .world_camera = glcamera_perspective(
            WORKBENCH_CAMERA_DEFAULT_POSITION,
            WORKBENCH_CAMERA_DEFAULT_ROTATION 
        ),
        .player_camera_position = vec3f(0.f),
        .draw_lines = list_init(line_t),
        .lightsources = list_init(gllight_t *),
        .render_config = {
            .wireframe_mode = false
        },
        .gui = {
            .handle = gui_init(
                arena, 
                (ui_region_t) {
                    .cursor = {0},
                    .width = global_window->width, 
                    .height = global_window->height
            }),
            .enable = true
        }
    };

    assetmanager_load_all_primitives(&global_poggen->systems.assets);

    glcamera_set_scroll_speed(&o.world_camera, 100.0f);
    gui_set_composition(&o.gui.handle, (ui_composition)workbench_compose_ui);

    return o;
}

void workbench_update_player_camera_position(workbench_t *self, const vec3f_t pos)
{
    self->player_camera_position = pos;
}

void workbench_pass_line(workbench_t *self, const line_t line) 
{
    list_append(&self->draw_lines, line);
}

void workbench_track_lightsource(workbench_t *self, const gllight_t *light)
{
    list_append_ptr(&self->lightsources, light);
}

void workbench_toggle_wireframe_mode(workbench_t *self)
{
    self->render_config.wireframe_mode = !self->render_config.wireframe_mode;
    //gui_set_wireframe_mode(self->gui.handle, self->render_config.wireframe_mode);
}

void workbench_toggle_gui(workbench_t *self)
{
    self->gui.enable = !self->gui.enable;
}

void workbench__internal_render_ui(workbench_t *self)
{
    gui_render(&self->gui.handle);
}

void workbench__internal_render_lightsources(workbench_t *self)
{
    if (self->lightsources.len == 0) 
        return;

    const list_t *lights = &self->lightsources;
    list_iterator(lights, iter) {
        glrenderer3d_draw((glrendererconfig_t) {
            .calls = {
                .count = 1,
                .call = {
                    [0] = {
                        .is_wireframe = self->render_config.wireframe_mode,
                        .textures = {0},
                        .vtx = (buffer_t){
                            .raw_data = (u8 *)DEFAULT_CUBE_VERTICES_8,
                            .size = sizeof(DEFAULT_CUBE_VERTICES_8)
                        },
                        .idx = {
                            .data = (u8 *)DEFAULT_CUBE_INDICES_8,
                            .nmemb = ARRAY_LEN(DEFAULT_CUBE_INDICES_8)
                        },
                        .attrs = {
                            .count = 1,
                            .attr = {
                                [0] = {
                                    .ncmp = 3,
                                    .interleaved = {0},
                                },
                            }
                        },
                        .shader_config = {
                            .shader = &self->shader,
                            .uniforms = {
                                .count = 4,
                                .data = {
                                    [0] = {
                                        .name = str_lit("view"),
                                        .value = glcamera_getview(&self->world_camera)
                                    },
                                    [1] = {
                                        .name = str_lit("projection"),
                                        .value = glms_perspective(
                                            radians(45), global_poggen->handle.app->window.aspect_ratio, 1.0f, 10000.0f)
                                    },
                                    [2] = {
                                        .name = str_lit("color"),
                                        .value.vec4 = ((gllight_t *)iter)->color
                                    },
                                    [3] = {
                                        .name = str_lit("transform"),
                                        .value = glms_mat4_mul(
                                            glms_translate_make(((gllight_t *)iter)->position),
                                            glms_scale_make((vec3f_t){10.f, 10.f, 10.f})
                                        ),
                                    },
                                }
                            }
                        }
                    }
                }
            } 
        });

    }
}

void workbench_destroy(workbench_t *self)
{
    glshader_destroy(&self->shader);
    glshader_destroy(&self->primitives.shader);
    spriteatlas_destroy(&self->primitives.atlas);
    list_destroy(&self->draw_lines);
    list_destroy(&self->lightsources);
    gui_destroy(&self->gui.handle);
}

void workbench__internal_render_batch_lines(workbench_t *self)
{
    if(!self->draw_lines.len) {
        return;
    }
    glrenderer3d_draw((glrendererconfig_t){
        .calls = {
            .count = 3,
            .call = {
                //camera
                [0] = {
                    .is_wireframe = true || self->render_config.wireframe_mode,
                    .textures = {0},
                    .idx = {
                        .data = (u8 *)&CAMERA_INDICES,
                        .nmemb = ARRAY_LEN(CAMERA_INDICES)
                    },
                    .vtx = (buffer_t){
                        .raw_data = (u8 *)&CAMERA_VERTICES,
                        .size = sizeof(CAMERA_VERTICES)
                    },
                    .textures = {0},
                    .attrs = {
                        .count = 1,
                        .attr = {
                        [0] = {
                            .ncmp = 3, 
                            .interleaved = {0}
                        }
                    },
                    },
                    .shader_config = {
                        .shader = &self->shader,
                        .uniforms = {
                            .count = 4,
                            .data = {
                                [0] = {
                                    .name = str_lit("view"),
                                    .value = glcamera_getview(&self->world_camera)
                                },
                                [1] = {
                                    .name = str_lit("projection"),
                                    .value = glms_perspective(
                                            radians(45), global_poggen->handle.app->window.aspect_ratio, 1.0f, 1000.0f
                                    )
                                },
                                [2] = {
                                    .name = str_lit("color"),
                                    .value.vec4 = COLOR_BLACK
                                },
                                [3] = {
                                    .name = str_lit("transform"),
                                    .value = glms_mat4_mul(
                                            glms_translate_make(self->player_camera_position),
                                            glms_scale_make((vec3f_t){10.f, 10.f, 10.f})
                                    ),
                                },
                            }
                        },
                    }
                },
                //platform
                [1] = {
                    .is_wireframe = self->render_config.wireframe_mode,
                    .textures = {0},
                    .idx = {
                        .data = (u8 *)&DEFAULT_CUBE_INDICES_8,
                        .nmemb = ARRAY_LEN(DEFAULT_CUBE_INDICES_8)
                    },
                    .vtx = (buffer_t){
                        .raw_data = (u8 *)&DEFAULT_CUBE_VERTICES_8,
                        .size = sizeof(DEFAULT_CUBE_VERTICES_8)
                    },
                    .textures = {0},
                    .attrs = {
                        .count = 1,
                        .attr = {
                            [0] = {
                                .ncmp = 3, 
                                .interleaved = {0}
                            }
                        },
                    },
                    .shader_config = {
                        .shader = &self->shader,
                        .uniforms = {
                            .count = 4,
                            .data = {
                                [0] = {
                                    .name = str_lit("view"),
                                    .value = glcamera_getview(&self->world_camera)
                                },
                                [1] = {
                                    .name = str_lit("projection"),
                                    .value = glms_perspective(
                                        radians(45), 
                                        global_poggen->handle.app->window.aspect_ratio, 
                                        1.0f, 1000.0f
                                    )
                                },
                                [2] = {
                                    .name = str_lit("transform"),
                                    .value = glms_scale(MATRIX4F_IDENTITY, (vec3f_t){1000.0f, 1.0f, 1000.0f}),
                                },
                                [3] = {
                                    .name = str_lit("color"),
                                    .value.vec4 = COLOR_DARK_GRAY
                                },
                            }
                        } 
                    }
                },
                // lines
                [2] = {
                    .draw_mode = GL_LINE,
                    .is_wireframe = self->render_config.wireframe_mode,
                    .textures = {0},
                    .shader_config = {
                        .shader = &self->shader,
                        .uniforms = {
                            .count = 4,
                            .data = {
                                [0] = {
                                    .name = str_lit("view"),
                                    .value = glcamera_getview(&self->world_camera)
                                },
                                [1] = {
                                    .name = str_lit("projection"),
                                    .value = glms_perspective(
                                        radians(45), 
                                        global_poggen->handle.app->window.aspect_ratio, 
                                        1.0f, 1000.0f
                                    )
                                },
                                [2] = {
                                    .name = str_lit("transform"),
                                    .value = MATRIX4F_IDENTITY
                                },
                                [3] = {
                                    .name = str_lit("color"),
                                    .value.vec4 = COLOR_BLUE
                                },
                            }
                        } 
                    },
                    .idx = {0},
                    .vtx = (buffer_t){
                        .raw_data = list_get_buffer(&self->draw_lines),
                        .size = list_get_size(&self->draw_lines),
                    },
                    .attrs = {
                        .count = 1,
                        .attr = {
                            // position
                            [0] = {
                                .ncmp = 3,
                                .interleaved = {0},
                            },
                        }
                    }
                }
            },
        },
    });
}

void workbench__internal_update_ui(workbench_t * const self);

void workbench_render(workbench_t *self)
{
    workbench__internal_update_ui(self);

    workbench__internal_render_grid(
        &self->shader,
        glcamera_getview(&self->world_camera),
        glms_perspective(
            radians(45), 
            global_poggen->handle.app->window.aspect_ratio, 
            1.0f, 1000.0f),
        self->world_camera.position
    );

    workbench__internal_render_batch_lines(self);

    workbench__internal_render_lightsources(self);

    if (self->gui.enable) {
        workbench__internal_render_ui(self);
    }

    list_clear(&self->draw_lines);
}

void workbench_update_world_camera(workbench_t * const self, const f32 dt)
{
    ASSERT(self);
    glcamera_process_input(&self->world_camera, dt);
}

void workbench__internal_update_ui(workbench_t * const self)
{
    if (!self->gui.enable) return;

    gui_update(&self->gui.handle, self->world_camera.position);
}

void workbench_render_push_cube(
    workbench_t * const self,
    const vec3f_t translation,
    const vec3f_t scale,
    const vec4f_t color,
    const bool override_wireframe
) {
    renderqueue_t * const renderqueue = &global_poggen->systems.renderqueue;
    const assetmanager_t * const assetmanager = &global_poggen->systems.assets;
    const matrix4f_t perspective_projection  = glms_perspective(
        radians(45), 
        global_poggen->handle.app->window.aspect_ratio, 
        1.0f, 
        10000.0f
    );

    const rendercommand_instance_t instance = {
        .translation = { translation.x, translation.y, translation.z, 0.f },
        .scale = { scale.x, scale.y, scale.z, 0.f },
        .color = color,
        .uv = spriteatlas_get_sprite(&self->primitives.atlas, PROTOTYPE_SPRITE_CHECKERED_DARK_GRAY),
    };

    rendercommand_t rendercommand = {
        .enable_wireframe = override_wireframe || self->render_config.wireframe_mode,
        .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
        .mesh = assetmanager_get_gpu_loaded_primitive_asset(assetmanager, GL_MESH_PRIMITIVE_TYPE_CUBE),
        .instance = {
            .raw_data = {0},
            .size = sizeof(rendercommand_instance_t)
        },
        .material = {
            .textures = {
                .count = 1,
                .ids = {
                    self->primitives.atlas.texture.id
                }
            },
            .shader = {
                .data = &self->primitives.shader,
                .uniforms = {
                    .count = 2,
                    .data = {
                       [0] = {
                           .name = str("projection"),
                           .value = perspective_projection
                       },
                       [1] = {
                           .name = str("view"),
                           .value = glcamera_getview(&self->world_camera)
                       }
                    }
                }
            }
        },
    };

    memcpy(rendercommand.instance.raw_data, &instance, sizeof(instance));
    renderqueue_pass_command(renderqueue, rendercommand);
}


