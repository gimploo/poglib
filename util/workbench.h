#pragma once
#include <poglib/gui.h>
#include <poglib/input/commandqueue.h>
#include <poglib/input/commandregistry.h>
#include <poglib/gfx/glrenderer2d.h>
#include <poglib/gfx/glrenderer3d.h>
#include <poglib/util/spriteatlas.h>

#include "./glcamera.h"
#include "./workbench/workbench-constants.h"
#include "./gllight.h"
#include "./workbench/ui/workbench-ui.h"
#include "./workbench/workbench-grid.h"
#include "poglib/application/window/sdl_window.h"
#include "poglib/ecs.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component/types.h"
#include "poglib/math/common.h"
#include "poglib/math/la.h"

typedef struct {

    vec3f_t start;
    vec3f_t end;

} line_t;

typedef enum workbench_action_type {
    WORKBENCH_ACTION_TYPE_CAMERA_DRAG_LOOK    = 0,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN      = 1,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT     = 2,
    WORKBENCH_ACTION_TYPE_COUNT
} workbench_action_type;

typedef struct {

    bool is_active;

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
    vec3f_t player_camera_position;

    // Lines that draws - this clears up after every render
    list_t draw_lines;

    list_t lightsources;

    commandqueue_t commandqueue;

    struct {
        u32 entity_id;
        glcamera_t *handle;
    } world_camera;

} workbench_t;

workbench_t workbench_init(arena_t * const arena);
void        workbench_tick(workbench_t *const self);
void        workbench_update(workbench_t *const self, const f32 dt);

void        workbench_toggle(workbench_t *const self);

void        workbench_pass_line(workbench_t *self, const line_t line);
void        workbench_track_lightsource(workbench_t *self, const gllight_t *light);

void        workbench_render_cube(workbench_t * const self, const vec3f_t translation, const vec3f_t scale, const vec4f_t color, const bool override_wireframe);
void        workbench_render(workbench_t *self);

void        workbench_destroy(workbench_t *self);


#define WORKBENCH_CAMERA_DEFAULT_POSITION (vec3f_t){0.f, 0.f, 10.f}
#define WORKBENCH_CAMERA_DEFAULT_ROTATION (vec2f_t){0}

void workbench__internal_worldcamera_input_handler(ecs_component_input_state_t * const state, const u16 command_bitmask, const f32 dt)
{
    const u16 bitmask = command_bitmask;
    if (!bitmask) return;

    const bool drag_look = bitmask & (1 << WORKBENCH_ACTION_TYPE_CAMERA_DRAG_LOOK);
    const bool zoom_in   = bitmask & (1 << WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN);
    const bool zoom_out  = bitmask & (1 << WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT);

    const f32 drag_sensitivity = 0.3f;
    const f32 zoom_sensitivity = 50.0f;

    vec2f_t euler_angle  = {0};
    f32 z_offset         = 0.f;

    if (drag_look) {
        const vec2i_t mouse_pos_delta = window_mouse_get_relative_position(global_window);
        euler_angle.x = radians((f32)mouse_pos_delta.y * drag_sensitivity);
        euler_angle.y = radians((f32)mouse_pos_delta.x * drag_sensitivity);
    }

    if (zoom_in) {
        z_offset = zoom_sensitivity * dt;
    }

    if(zoom_out) {
        z_offset = -1.f * zoom_sensitivity * dt;
    }

    state->rotation = (vec3f_t){ euler_angle.x, euler_angle.y };
    state->movement = (vec3f_t){ 0.f, 0.f, z_offset };
}

void workbench__internal_ecs_create_world_camera(workbench_t *const self)
{
    const u32 world_camera_entity_id  = ecs_entity_add(
        &global_poggen->systems.ecs, 
        (ecs_componentbundle_t) {
            .signature = ECS_CMP_TRANSFORM | ECS_CMP_CAMERA | ECS_CMP_INPUT,
            .component = {
                [ECS_CMP_TRANSFORM_IDX].transform = {0},
                [ECS_CMP_CAMERA_IDX].camera = (ecs_component_camera_t){
                    .camera = glcamera_perspective((vec3f_t){2.0f, 4.f, -4.0f}, (vec2f_t){-0.32f, 1.59f}),
                    .mode = ECS_CMP_CAMERA_MODE_FREE_FLY,
                },
                [ECS_CMP_INPUT_IDX].input = (ecs_component_input_t){
                    .input_behavior = workbench__internal_worldcamera_input_handler
                }
            }
        }
    );

    const ecs_entity_view_t view = ecs_entity_query_components(
        &global_poggen->systems.ecs, world_camera_entity_id, ECS_CMP_CAMERA);
    ASSERT(view.entity_cmp_data[ECS_CMP_CAMERA_IDX]);
    self->world_camera.handle = &((ecs_component_camera_t *)view.entity_cmp_data[ECS_CMP_CAMERA_IDX])->camera;
    self->world_camera.entity_id = world_camera_entity_id;
}

workbench_t workbench_init(arena_t *const arena)
{
    ASSERT(global_poggen);

    if (!global_poggen->config.enable_ecs) {
        eprint("ECS required to use workbench");
    }

    workbench_t o = {
        .is_active = false,
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
        },
        .commandqueue = commandqueue(arena, (commandregistry_t){
            .count = WORKBENCH_ACTION_TYPE_COUNT,
            .registry = {
                [WORKBENCH_ACTION_TYPE_CAMERA_DRAG_LOOK] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse = {
                        .key        = SDL_MOUSEBUTTON_LEFT,
                        .trigger    = SDL_MOUSESTATE_DRAG,
                    }
                },
                [WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse.wheel = SDL_MOUSEWHEEL_UP,
                },
                [WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse.wheel = SDL_MOUSEWHEEL_DOWN,
                }
            },
        })
    };

    workbench__internal_ecs_create_world_camera(&o);

    assetmanager_load_all_primitives(&global_poggen->systems.assets);

    gui_set_composition(&o.gui.handle, (ui_composition)workbench_compose_ui);

    return o;
}

matrix4f_t workbench__internal_get_camera_view(const workbench_t *const self)
{
    glcamera_t *camera = ecs_get_active_camera(&global_poggen->systems.ecs);
    return glcamera_getview(camera);
}

void workbench_pass_line(workbench_t *self, const line_t line) 
{
    list_append(&self->draw_lines, line);
}

void workbench_track_lightsource(workbench_t *self, const gllight_t *light)
{
    list_append_ptr(&self->lightsources, light);
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
                                        .value = workbench__internal_get_camera_view(self)
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
                                    .value = workbench__internal_get_camera_view(self)
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
                                    .value = workbench__internal_get_camera_view(self)
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
                                    .value = workbench__internal_get_camera_view(self)
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

void workbench__internal_update_ui(workbench_t * const self, const vec3f_t worlcamera_position);

void workbench_render(workbench_t *self)
{
    workbench__internal_update_ui(self, self->world_camera.handle->position);

    workbench__internal_render_grid(
        &self->shader,
        workbench__internal_get_camera_view(self),
        glms_perspective(
            radians(45), 
            global_poggen->handle.app->window.aspect_ratio, 
            1.0f, 1000.0f),
        self->world_camera.handle->position
    );

    workbench__internal_render_batch_lines(self);

    workbench__internal_render_lightsources(self);

    if (self->gui.enable) {
        workbench__internal_render_ui(self);
    }

    list_clear(&self->draw_lines);
}

void workbench__internal_update_ui(workbench_t * const self, const vec3f_t world_camera_position)
{
    if (!self->gui.enable) return;

    gui_update(&self->gui.handle, world_camera_position);
}

void workbench_render_camera(
    workbench_t * const self,
    const vec3f_t position,
    const vec3f_t orientation
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
        .translation = { position.x, position.y, position.z, 0.f },
        .orientation = { orientation.x, orientation.y, orientation.z, 0.f }, 
        .scale = vec4f(0.5f),
        .color = COLOR_GRAY,
    };

    gpu_mesh_t * const mesh = assetmanager_get_gpu_loaded_primitive_asset_async(assetmanager, GL_MESH_PRIMITIVE_TYPE_CAMERA);
    if(!mesh) {
        return;
    }

    rendercommand_t rendercommand = {
        .enable_wireframe = true,
        .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
        .mesh = mesh,
        .instance = {
            .raw_data = {0},
            .size = sizeof(rendercommand_instance_t)
        },
        .material = {
            .textures = {0},
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
                           .value = workbench__internal_get_camera_view(self)
                       }
                    }
                }
            }
        },
    };

    memcpy(rendercommand.instance.raw_data, &instance, sizeof(instance));
    renderqueue_pass_command(renderqueue, rendercommand);
}

void workbench_render_line(
    workbench_t * const self,
    const vec3f_t starting_pos,
    const vec3f_t ending_pos
) {
    
}


void workbench_render_marker(
    workbench_t * const self,
    const vec3f_t translation,
    const vec4f_t color
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
        .scale = vec4f(0.05f),
        .orientation = vec4f(0.f),
        .color = color,
        .uv = spriteatlas_get_sprite(&self->primitives.atlas, PROTOTYPE_SPRITE_YELLOW_T),
    };

    gpu_mesh_t * const mesh = assetmanager_get_gpu_loaded_primitive_asset_async(assetmanager, GL_MESH_PRIMITIVE_TYPE_CUBE);
    if(!mesh) {
        return;
    }

    rendercommand_t rendercommand = {
        .enable_wireframe = self->render_config.wireframe_mode,
        .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
        .mesh = mesh,
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
                           .value = workbench__internal_get_camera_view(self)
                       }
                    }
                }
            }
        },
    };

    memcpy(rendercommand.instance.raw_data, &instance, sizeof(instance));
    renderqueue_pass_command(renderqueue, rendercommand);
}


void workbench_render_cube(
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
        .orientation = vec4f(0.f),
        .color = color,
        .uv = spriteatlas_get_sprite(&self->primitives.atlas, PROTOTYPE_SPRITE_CHECKERED_DARK_GRAY),
    };

    gpu_mesh_t * const mesh = assetmanager_get_gpu_loaded_primitive_asset_async(assetmanager, GL_MESH_PRIMITIVE_TYPE_CUBE);
    if(!mesh) {
        return;
    }

    rendercommand_t rendercommand = {
        .enable_wireframe = override_wireframe || self->render_config.wireframe_mode,
        .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
        .mesh = mesh,
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
                           .value = workbench__internal_get_camera_view(self)
                       }
                    }
                }
            }
        },
    };

    memcpy(rendercommand.instance.raw_data, &instance, sizeof(instance));
    renderqueue_pass_command(renderqueue, rendercommand);
}

void workbench_tick(workbench_t *const self)
{
    if (!self->is_active) return;

    commandqueue_sync(&self->commandqueue);
}

void workbench_update(workbench_t *const self, const f32 dt)
{
    if (!self->is_active) return;

    //NOTE:keep commandqueue flush at the end
    commandqueue_flush(&self->commandqueue);
}

void workbench_toggle(workbench_t *const self)
{
    self->is_active = !self->is_active;

    ecs_patch_entity(
        &global_poggen->systems.ecs, 
        self->world_camera.entity_id, 
        (ecs_cmp_patch_payload_t){
            .patch_type = ECS_PATCH_CMP_ACTIVE_FIELD,
            .signature = ECS_CMP_CAMERA | ECS_CMP_INPUT | ECS_CMP_TRANSFORM,
            .is_active = self->is_active
        }
    );

    if (!self->is_active) return;

    ecs_set_active_camera(
        &global_poggen->systems.ecs, 
        self->world_camera.entity_id
    );

    ecs_set_active_commandqueue(
        &global_poggen->systems.ecs, 
        &self->commandqueue
    );
}

