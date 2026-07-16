#pragma once
#include <poglib/poggen.h>
#include <poglib/ecs.h>

#include "./workbench/common.h"
#include "./workbench/workbench-grid.h"
#include "poglib/ecs/component/types.h"
#include "poglib/util/workbench/workbench-editor.h"

workbench_t *   workbench_init(arena_t * const arena);
void            workbench_ecs_populate_entities(void);
void            workbench_update(const f32 dt);
void            workbench_render(void);
void                workbench_toggle(void);
void            workbench_destroy(void);


#define WORKBENCH_CAMERA_DEFAULT_POSITION (vec3f_t){0.f, 0.f, 10.f}
#define WORKBENCH_CAMERA_DEFAULT_ROTATION (vec2f_t){0}

INTERNAL void workbench__internal__show_colliders(workbench_t *const self);

INTERNAL void workbench__internal__worldcamera_input_handler(ecs_component_input_state_t *const state, const u16 bitmask, const f32 dt)
{

    //NOTE: zoom_in, zoom_out and panning wont work for oribiting camera since the cameras position is overwritten by the offset logic 
    //in the camera system - just an FYI

    const bool drag_look    = bitmask & (1 << WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_DRAG);
    const bool panning      = bitmask & (1 << WORKBENCH_ACTION_TYPE_MOUSE_MIDDLE_CLICK_DRAG);
    const bool zoom_in      = bitmask & (1 << WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN);
    const bool zoom_out     = bitmask & (1 << WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT);
    const bool zoom_fast    = bitmask & ((1 << WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN_FAST) | (1 << WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT_FAST));

    const f32 drag_sensitivity      = 0.3f;
    const f32 zoom_sensitivity      = !zoom_fast ? 50.0f : 200.f;
    f32 z_offset                    = 0.f;
    const vec2i_t mouse_rel         = window_mouse_get_relative_position(global_window);

    if (panning) {
        const f32 pane_sensitivity   = 2.f * dt;
        const vec2s mouse_rel_offset = (vec2s) { 
            (f32)mouse_rel.x * pane_sensitivity * -1.f,
            (f32)mouse_rel.y * pane_sensitivity
        };
        state->current_position = glms_vec3_add(
            state->current_position, 
            glms_vec3_add(
                glms_vec3_scale(state->right,   mouse_rel_offset.x),
                glms_vec3_scale(state->up,      mouse_rel_offset.y)
            )
        );
        return;
    }

    //NOTE: Zoom in and out
    {
        if (zoom_in)                z_offset = 1.f * zoom_sensitivity * dt;
        if (zoom_out)               z_offset = -1.f * zoom_sensitivity * dt;

        if (zoom_in || zoom_out)
        {
            ecs_component_camera_t *const camera_cmp = ecs_entity_query_components(
                global_ecs, global_workbench->world_camera.entity_id, ECS_CMP_CAMERA
            ).entity_cmp_data[ECS_CMP_CAMERA_IDX];

            if (camera_cmp->mode == ECS_CMP_CAMERA_MODE_ORBIT_FOLLOW) 
            {
                ecs_component_transform_t *target_transform = ecs_entity_query_components(
                    global_ecs, camera_cmp->follow.track_entity_id, ECS_CMP_TRANSFORM
                ).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

                ecs_component_transform_t *const camera_transform = ecs_entity_query_components(
                    global_ecs, global_workbench->world_camera.entity_id, ECS_CMP_TRANSFORM
                ).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

                const f32 distance = glms_vec3_distance(target_transform->position, camera_transform->position);
                if (distance > 2.f || zoom_out) {
                    camera_cmp->follow.orbit_radius += z_offset;
                    state->current_position = glms_vec3_add(state->current_position, glms_vec3_scale(state->front, z_offset));
                }

            } else {

                state->current_position = glms_vec3_add(state->current_position, glms_vec3_scale(state->front, z_offset));
            }
        }
    }


    if (drag_look) {

        static f32 pitch    = 0.f; 
        static f32 yaw      = 0.f;

        const vec2f_t mouse_delta = {
            .x = radians((f32)mouse_rel.x * drag_sensitivity),
            .y = radians((f32)mouse_rel.y * drag_sensitivity),
        };

        pitch   += mouse_delta.y;
        yaw     += mouse_delta.x;

        pitch = glm_clamp(pitch, radians(-89), radians(89));

        const versors pitch_quat = glms_quatv(pitch, (vec3s){-1.f, 0.f, 0.f});
        const versors yaw_quat = glms_quatv(yaw, (vec3s){0.f, -1.f, 0.f});

        state->current_orientation = glms_quat_mul(yaw_quat, pitch_quat);
    }

#if 0
    printf("front       ");glms_vec3_print(state->front, stdout);
    printf("right       ");glms_vec3_print(state->right, stdout);
    printf("up          ");glms_vec3_print(state->up, stdout);
    printf("orientation ");glms_versor_print(state->current_orientation, stdout);
#endif

}

INTERNAL void workbench__internal__ecs_create_world_camera(workbench_t *const self)
{
    const u32 world_camera_entity_id  = ecs_entity_add(
        global_ecs,
        (ecs_componentbundle_t) {
            .signature = ECS_CMP_TRANSFORM | ECS_CMP_CAMERA | ECS_CMP_INPUT,
            .component = {
                [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                    .orientation = GLMS_QUAT_IDENTITY_INIT,
                    .source = ECS_CMP_TRANSFORM_SOURCE_INPUT,
                },
                [ECS_CMP_CAMERA_IDX].camera = (ecs_component_camera_t){
                    .camera = glcamera_perspective((vec3f_t){2.0f, 4.f, -4.0f}, (vec2f_t){-0.32f, 1.59f}),
                    .mode = ECS_CMP_CAMERA_MODE_FREE_FLY,
                },
                [ECS_CMP_INPUT_IDX].input = (ecs_component_input_t ){
                    .input_behavior = workbench__internal__worldcamera_input_handler
                }
            }
        }
    );
    ASSERT(world_camera_entity_id == WORKBENCH_RESERVED_ENTITY_ID_WORLDCAMERA);

    const ecs_entity_query_t view = ecs_entity_query_components(global_ecs, world_camera_entity_id, ECS_CMP_CAMERA);
    ASSERT(view.entity_cmp_data[ECS_CMP_CAMERA_IDX]);
    self->world_camera.handle       = &((ecs_component_camera_t *)view.entity_cmp_data[ECS_CMP_CAMERA_IDX])->camera;
    self->world_camera.entity_id    = world_camera_entity_id;

    ecs_patch_entity(
        global_ecs, 
        world_camera_entity_id, 
        (ecs_cmp_patch_payload_t) {
            .patch_type = ECS_PATCH_CMP_ACTIVE_FIELD,
            .is_active = false,
            .signature = ECS_CMP_TRANSFORM
        }
    );
}

void workbench_ecs_populate_entities(void)
{
    ASSERT(global_workbench);
    ASSERT(global_ecs);
    workbench__internal__ecs_create_world_camera(global_workbench);
}

workbench_t * workbench_init(arena_t *const arena)
{
    ASSERT(global_engine);
    ASSERT(!global_workbench);

    assetmanager_t *const assetmanager = &global_engine->systems.assets;

    workbench_t workbench = {
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
            .mesh_shader_id = assetmanager_load_glsl_shader(
                assetmanager,
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
                }
            ),
            .line_shader_id = assetmanager_load_glsl_shader(
                assetmanager,
                str(POGLIB_ROOT_DIR"/pipeline/render/shader/instance-line-vtx.glsl"),
                str(POGLIB_ROOT_DIR"/pipeline/render/shader/instance-line-frag.glsl"),
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
                }
            ),
            .atlas_id = assetmanager_load_spriteatlas(assetmanager, str(POGLIB_ROOT_DIR"/res/sprites/prototype.png"), 8, 4),
        },
        .player_camera_position = vec3f(0.f),
        .render_config = {
            .wireframe_mode = false
        },
        .world_camera = {0},
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
        .editor = {
            .workbench_editor_action_history = stack_init(100, workbench_editor_ecs_action_t, arena),
        },
        .commandregistry = (commandregistry_t){
            .count = WORKBENCH_ACTION_TYPE_COUNT,
            .registry = {
                [WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_DRAG] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse = {
                        .key        = SDL_MOUSEBUTTON_LEFT,
                        .trigger    = SDL_MOUSESTATE_DRAG,
                    }
                },
                [WORKBENCH_ACTION_TYPE_MOUSE_MIDDLE_CLICK_DRAG] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse = {
                        .key        = SDL_MOUSEBUTTON_MIDDLE,
                        .trigger    = SDL_MOUSESTATE_DRAG,
                    }
                },
                [WORKBENCH_ACTION_TYPE_MOUSE_KEYBOARD_UNSELECT_ENTITY] = {
                    .type = COMMANDINPUTKEY_TYPE_KEYBOARD,
                    .sdl_keyboard_key = {
                        .main = SDL_SCANCODE_CAPSLOCK,
                        .trigger = COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED
                    }
                },
                [WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_JUST_CLICKED] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse = {
                        .key        = SDL_MOUSEBUTTON_LEFT,
                        .trigger    = SDL_MOUSESTATE_JUST_PRESSED,
                    }
                },
                [WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse.wheel = SDL_MOUSEWHEEL_UP,
                },
                [WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse.wheel = SDL_MOUSEWHEEL_DOWN,
                },
                [WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN_FAST] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse = {
                        .wheel = SDL_MOUSEWHEEL_UP,
                        .modifier = SDL_SCANCODE_LSHIFT,
                    }
                },
                [WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT_FAST] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse = {
                        .wheel = SDL_MOUSEWHEEL_DOWN,
                        .modifier = SDL_SCANCODE_LSHIFT,
                    }
                },
                [WORKBENCH_ACTION_TYPE_TOGGLE_WIREFRAME] = {
                    .type = COMMANDINPUTKEY_TYPE_KEYBOARD,
                    .sdl_keyboard_key = {
                        .main = SDL_SCANCODE_TAB,
                        .trigger = COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED
                    }
                },
                [WORKBENCH_ACTION_TYPE_MOUSE_ENTITY_SELECTION] = {
                    .type = COMMANDINPUTKEY_TYPE_MOUSE,
                    .sdl_mouse = {
                        .key = SDL_MOUSEBUTTON_RIGHT,
                        .trigger= SDL_MOUSESTATE_JUST_PRESSED,
                    },
                },
                [WORKBENCH_ACTION_TYPE_KEYBOARD_COPYPASTE_ENTITY] = {
                    .type = COMMANDINPUTKEY_TYPE_KEYBOARD,
                    .sdl_keyboard_key = {
                        .modifier = SDL_SCANCODE_LCTRL,
                        .main = SDL_SCANCODE_C,
                        .trigger = COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED
                    }
                },
                [WORKBENCH_ACTION_TYPE_KEYBOARD_DELETE_ENTITY] = {
                    .type = COMMANDINPUTKEY_TYPE_KEYBOARD,
                    .sdl_keyboard_key = {
                        .main = SDL_SCANCODE_DELETE,
                        .trigger = COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED
                    }
                },
                [WORKBENCH_ACTION_TYPE_UNDO] = {
                    .type = COMMANDINPUTKEY_TYPE_KEYBOARD,
                    .sdl_keyboard_key = {
                        .modifier   = SDL_SCANCODE_LCTRL,
                        .main       = SDL_SCANCODE_Z,
                        .trigger    = COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED
                    }
                },
                [WORKBENCH_ACTION_TYPE_SAVE] = {
                    .type = COMMANDINPUTKEY_TYPE_KEYBOARD,
                    .sdl_keyboard_key = {
                        .modifier   = SDL_SCANCODE_LCTRL,
                        .main       = SDL_SCANCODE_S,
                        .trigger    = COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED
                    }
                },
                [WORKBENCH_ACTION_TYPE_EXPORT_GLB] = {
                    .type = COMMANDINPUTKEY_TYPE_KEYBOARD,
                    .sdl_keyboard_key = {
                        .modifier   = SDL_SCANCODE_LCTRL,
                        .main       = SDL_SCANCODE_E,
                        .trigger    = COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED
                    }
                }
            },
        }
    };


    assetmanager_load_all_primitives(&global_engine->systems.assets);

    global_workbench = arena_store(arena, &workbench, sizeof(workbench));

    workbench_ecs_populate_entities();
    workbench_debug_renderer_init(&global_workbench->debug_renderer, arena);

    return global_workbench;
}

INTERNAL matrix4f_t workbench__internal__get_camera_view(const workbench_t *const self)
{
    glcamera_t *camera = ecs_get_active_camera(global_ecs);
    return glcamera_getview(camera);
}



#if 0
void workbench__internal_render_lightsources(workbench_t *self)
{
    const list_t *lights = {0};
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
#endif

void workbench_destroy(void)
{
    ASSERT(global_workbench);
    workbench_t *self = global_workbench;

    workbench_debug_renderer_destroy(&self->debug_renderer);
    glshader_destroy(&self->shader);
    gui_destroy(&self->gui.handle);

    global_workbench = NULL;
}

void workbench_render(void)
{
    ASSERT(global_workbench);
    workbench_t *self = global_workbench;

    if (self->enable_collider) {
        workbench__internal__show_colliders(self);
    }

    if (!self->is_active) return;


    if (!self->disable_grid) {
        workbench__internal_render_grid(
            &self->shader,
            workbench__internal__get_camera_view(self),
            glms_perspective(
                radians(45), 
                global_engine->handle.app->window.aspect_ratio, 
                1.0f, 1000.0f),
            self->world_camera.handle->position
        );
    }

//    workbench__internal_render_lightsources(self);

    workbench_editor_render();
}


void workbench_render_camera(
    const vec3f_t position,
    const versors orientation
) {
    ASSERT(global_workbench);
    renderqueue_t * const renderqueue = &global_engine->systems.renderqueue;
    const assetmanager_t * const assetmanager = &global_engine->systems.assets;
    const matrix4f_t perspective_projection  = glms_perspective(
        radians(45), 
        global_engine->handle.app->window.aspect_ratio, 
        1.0f, 
        10000.0f
    );


    gpu_asset_t *const asset = assetmanager_get_gpu_loaded_asset_async(assetmanager, GL_MESH_PRIMITIVE_TYPE_CAMERA);
    if(!asset) {
        return;
    }
    ASSERT(asset->meshes.count);

    rendercommand_t rendercommand = {
        .enable_wireframe = true,
        .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
        .mesh = asset->meshes.data,
        .instance = {
            .raw_data = &(rendercommand_instance_primitive_mesh_t) {
                .translation = { position.x, position.y, position.z, 0.f },
                .orientation = { orientation.x, orientation.y, orientation.z, orientation.w }, 
                .scale = vec4f(0.5f),
                .color = COLOR_GRAY,
            },
            .size = sizeof(rendercommand_instance_primitive_mesh_t)
        },
        .material = {
            .texture = {0},
            .shader = {
                .data = assetmanager_get_assetresource(assetmanager, ASSET_TYPE_GLSL_SHADER, global_workbench->primitives.mesh_shader_id),
                .uniforms = {
                    .count = 2,
                    .data = {
                       [0] = {
                           .name = str("projection"),
                           .value = perspective_projection
                       },
                       [1] = {
                           .name = str("view"),
                           .value = workbench__internal__get_camera_view(global_workbench)
                       }
                    }
                }
            }
        },
    };

    renderqueue_pass_command(renderqueue, rendercommand);
}



void workbench_render_marker(
    const vec3f_t translation,
    const vec4f_t color
) {
    ASSERT(global_workbench);
    workbench_t *const self = global_workbench;
    renderqueue_t * const renderqueue = &global_engine->systems.renderqueue;
    const assetmanager_t * const assetmanager = &global_engine->systems.assets;
    const matrix4f_t perspective_projection  = glms_perspective(
        radians(45), 
        global_engine->handle.app->window.aspect_ratio, 
        1.0f, 
        10000.0f
    );

    spriteatlas_t *atlas = (spriteatlas_t *)assetmanager_get_assetresource(
        assetmanager, ASSET_TYPE_TEXTURE_SPRITE_ATLAS, self->primitives.atlas_id);


    gpu_asset_t * const asset = assetmanager_get_gpu_loaded_asset_async(assetmanager, GL_MESH_PRIMITIVE_TYPE_CUBE);
    if(!asset) {
        return;
    }

    rendercommand_t rendercommand = {
        .enable_wireframe = self->render_config.wireframe_mode,
        .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
        .mesh = asset->meshes.data,
        .instance = {
            .raw_data = &(rendercommand_instance_primitive_mesh_t) {
                .translation = { translation.x, translation.y, translation.z, 0.f },
                .scale = vec4f(0.05f),
                .orientation = {0.f, 0.f, 0.f, 1.f},
                .color = color,
                .uv = spriteatlas_get_sprite(atlas, PROTOTYPE_SPRITE_YELLOW_T),
            },
            .size = sizeof(rendercommand_instance_primitive_mesh_t)
        },
        .material = {
            .texture = {
                .count = 1,
                .items = {
                    [0] = (gltextureitem_t){
                        .type = GL_TEXTURE_TYPE_NORMAL,
                        .source = { .normal_texture = &atlas->texture }
                    }
                }
            },
            .shader = {
                .data = assetmanager_get_assetresource(assetmanager, ASSET_TYPE_GLSL_SHADER, self->primitives.mesh_shader_id),
                .uniforms = {
                    .count = 2,
                    .data = {
                       [0] = {
                           .name = str("projection"),
                           .value = perspective_projection
                       },
                       [1] = {
                           .name = str("view"),
                           .value = workbench__internal__get_camera_view(self)
                       }
                    }
                }
            }
        },
    };

    renderqueue_pass_command(renderqueue, rendercommand);
}

void workbench_update(const f32 dt)
{
    if (!global_workbench->is_active) return;

    const u32 bitmask = commandqueue_get_commands_as_bitmask(&global_engine->systems.commandqueue);

    if (bitmask & (1 << WORKBENCH_ACTION_TYPE_MOUSE_ENTITY_SELECTION))          workbench_editor_select_closest_entity();
    if (bitmask & (1 << WORKBENCH_ACTION_TYPE_MOUSE_KEYBOARD_UNSELECT_ENTITY))  workbench_editor_savechanges();
    if (bitmask & (1 << WORKBENCH_ACTION_TYPE_TOGGLE_WIREFRAME))                global_workbench->render_config.wireframe_mode = !global_workbench->render_config.wireframe_mode;
    if (bitmask & (1 << WORKBENCH_ACTION_TYPE_KEYBOARD_COPYPASTE_ENTITY))       workbench_editor_copypaste_entity();
    if (bitmask & (1 << WORKBENCH_ACTION_TYPE_KEYBOARD_DELETE_ENTITY))          workbench_editor_delete_entity();
    if (bitmask & (1 << WORKBENCH_ACTION_TYPE_UNDO))                            workbench_editor_action_history_pop(global_workbench, global_ecs);
    if (bitmask & (1 << WORKBENCH_ACTION_TYPE_SAVE))                            workbench_editor_save_to_file(global_workbench, global_ecs);
    if (bitmask & (1 << WORKBENCH_ACTION_TYPE_EXPORT_GLB))                      workbench_editor_export_glb(global_ecs);

    workbench_editor_update();
}

void workbench_toggle(void)
{
    ASSERT(global_workbench);
    workbench_t *self = global_workbench;

    self->is_active = !self->is_active;
    logging("Workbench toggled status %s", self->is_active ? "enabled" : "disabled");

    ecs_patch_entity(
        global_ecs,
        self->world_camera.entity_id, 
        (ecs_cmp_patch_payload_t){
            .patch_type = ECS_PATCH_CMP_ACTIVE_FIELD,
            .signature = ECS_CMP_CAMERA | ECS_CMP_INPUT | ECS_CMP_TRANSFORM,
            .is_active = self->is_active
        }
    );

    //NOTE: when you have someting selected and immedialty go to playmode - we want the entity unselected and saved if 
    //there were any changes done it
    if (!self->is_active)   workbench_editor_savechanges();

    ecs_set_active_camera(
        global_ecs,
        self->world_camera.entity_id
    );

    poggen_update_commandqueue_registry(global_engine, self->commandregistry);
}

void workbench__internal__show_colliders(workbench_t *const self)
{
    ASSERT(global_engine);
    ASSERT(global_physics_sys_jolt_instance);

    if (!global_workbench->enable_collider) return;

    JPH_DrawSettings draw_settings;
    JPH_DrawSettings_InitDefault(&draw_settings);
    draw_settings.drawShape = true;
    draw_settings.drawShapeWireframe = true;
    draw_settings.drawShapeColor = JPH_BodyManager_ShapeColor_InstanceColor;

    JPH_PhysicsSystem_DrawBodies(
        global_physics_sys_jolt_instance->physics_system,
        &draw_settings,
        self->debug_renderer.jolt_handle,
        NULL
    );

    slot_t *sc_pool = slot_get_value(&global_ecs->managers.componentmanager.componentpool_slots, ECS_CMP_COLLIDER_IDX);
    slot_iterator(sc_pool, sc_iter)
    {
        ecs_component_poolentry_t *sc_entry = sc_iter;
        if (!sc_entry->is_active) continue;

        ecs_component_collider_t *sc = (ecs_component_collider_t *)sc_entry->entity_cmpdata;
        if (sc->motion_type != JPH_MotionType_Kinematic) continue;
        if (!sc->internal.kinematic_body) continue;

        const JPH_Shape *shape = JPH_CharacterBase_GetShape((JPH_CharacterBase *)sc->internal.kinematic_body);
        if (!shape) continue;

        JPH_Vec3 shape_offset;
        JPH_CharacterVirtual_GetShapeOffset(sc->internal.kinematic_body, &shape_offset);

        JPH_Vec3 draw_pos;
        draw_pos.x = sc->internal.position.x + shape_offset.x;
        draw_pos.y = sc->internal.position.y + shape_offset.y;
        draw_pos.z = sc->internal.position.z + shape_offset.z;

        JPH_Mat4 transform;
        JPH_Mat4_RotationTranslation(
            &transform,
            (JPH_Quat *)&sc->internal.orientation,
            &draw_pos
        );

        const JPH_Vec3 scale = {1.0f, 1.0f, 1.0f};
        JPH_Shape_Draw(
            shape,
            self->debug_renderer.jolt_handle,
            &transform,
            &scale,
            0xFFFFFFFF,
            false,
            true
        );
    }

    const matrix4f_t view = workbench__internal__get_camera_view(self);
    const matrix4f_t proj = glms_perspective(
        radians(45),
        global_engine->handle.app->window.aspect_ratio,
        1.0f,
        10000.0f
    );

    workbench_debug_renderer_flush(
        &self->debug_renderer,
        &self->shader,
        view,
        proj,
        self->world_camera.handle->position
    );
}

