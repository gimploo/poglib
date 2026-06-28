#pragma once
#include "poglib/basic/color.h"
#include "poglib/ecs/component/types.h"
#include "poglib/external/joltc/include/joltc.h"
#include "poglib/gui.h"
#include "poglib/physics/jolt-wrapper.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/util/asset.h"
#include "poglib/util/assetmanager.h"
#include <poglib/util/workbench/common.h>
#include <poglib/ecs.h>

INTERNAL f32 workbench_editor__internal_closest_point_on_ray(const vec3f_t ray_origin, const vec3f_t ray_dir, const vec3f_t targetpoint)
{
    const vec3f_t to_point = glms_vec3_sub(targetpoint, ray_origin);
    f32 t = glms_vec3_dot(to_point, ray_dir);
    if (t < 0.f) t = 0.f;
    vec3f_t closest = glms_vec3_add(ray_origin, glms_vec3_scale(ray_dir, t));
    return glms_vec3_distance(closest, targetpoint);
}

INTERNAL void workbench_editor__internal_check_mouse_closest_entity(void)
{
    vec2f_t ndc = window_mouse_get_norm_position(global_window);
    glcamera_t *cam = global_workbench->world_camera.handle;

    vec3f_t dir = {0};
    {
        const matrix4f_t view       = glcamera_getview(cam);
        const matrix4f_t proj       = glms_perspective(radians(45), global_engine->handle.app->window.aspect_ratio, 0.1f, 1000.0f);
        const matrix4f_t inv_pv     = glms_mat4_inv(glms_mat4_mul(proj, view));
        const vec4f_t near          = { ndc.x, ndc.y, -1.0f, 1.0f };
        const vec4f_t far           = { ndc.x, ndc.y,  1.0f, 1.0f };

        vec4f_t near_w      = glms_mat4_mulv(inv_pv, near);
        vec4f_t far_w       = glms_mat4_mulv(inv_pv, far);
        near_w              = glms_vec4_scale(near_w, 1.0f / near_w.w);
        far_w               = glms_vec4_scale(far_w,  1.0f / far_w.w);

        dir                 = glms_vec3_normalize(glms_vec3_sub(*(vec3f_t *)&far_w, *(vec3f_t *)&near_w));
    }

    u32 picked = 0;
    f32 closest_dist = 5.f;

    slot_iterator(&global_ecs->managers.entitymanager.entities, iter)
    {
        if (!slot_iterator_index) continue;

        const ecs_entity_t *const e         = iter;
        const ecs_entity_query_t q          = ecs_entity_query_components(global_ecs, e->id, ECS_CMP_TRANSFORM);
        const ecs_component_transform_t *t  = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        if (!t) continue;

        const f32 d = workbench_editor__internal_closest_point_on_ray(cam->position, dir, t->position);
        if (d < closest_dist) {
            closest_dist = d;
            picked = e->id;
        }
    }

    global_workbench->editor.mouse_closest_to_entity_id = picked;
}

INTERNAL void workbench_editor__internal_show_entity_info_for_selected_entity(void)
{
    if (!global_workbench->editor.current_selected_entity_id) return;

    gui_t *const gui = &global_workbench->gui.handle;
    char tempbuffer[32] = {0};

    enum option_type {
        OT_TRANSLATION  = 0,
        OT_ROTATION     = 1,
        OT_SCALE        = 2,
        OT_COUNT
    };

    const str_t transform_member_labels[OT_COUNT] = {
        [OT_TRANSLATION]    = str("Translation"),
        [OT_ROTATION]       = str("Rotation"),
        [OT_SCALE]          = str("Scale"),
    };

    ecs_component_transform_t *const transform = ecs_entity_query_components(
        global_ecs, global_workbench->editor.current_selected_entity_id, ECS_CMP_TRANSFORM
    ).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

    //NOTE: used in the editor to udpate value for each one of these
    vec3f_t *const transform_bindings[OT_COUNT] = {
        &transform->position,
        (vec3f_t *)&transform->orientation,
        &transform->scale,
    };

    gui_ui_compose_begin(gui, (ui_config_t) {
        .layout = UI_LAYOUT_VERTICAL,
        .dim = {
            .min_width = 300,
            .min_height = 250,
        },
        .margin = {
            .top = 5.f,
            .left = 5.f
        },
        .color = {
            .base = COLOR_ABYSS_BLUE
        }
    });
    {

        //INFO: ==Entity label=====================================================================================
        snprintf(tempbuffer, sizeof(tempbuffer), "Entity Id: %d", global_workbench->editor.current_selected_entity_id);
        gui_ui_compose_begin(gui, (ui_config_t){
            .composition = {
                .styles = UI_STYLE_ONLY_TEXT,
            },
            .dim = {
                .min_width  = 80,
                .min_height = 30,
            },
            .text = str__from_cstr(tempbuffer, sizeof(tempbuffer)),
            .color = {
                .base = COLOR_WHITE
            },
            .margin = {
                .top = 5.f,
                .left = 5.f,
            }
        });
        gui_ui_compose_end(gui);
        //INFO: =======================================================================================

        for (u8 idx = 0; idx < OT_COUNT; idx ++)
        {

            //NOTE: Container
            gui_ui_compose_begin(gui, (ui_config_t){
                .dim = {
                    .min_width = 300,
                    .min_height = 50,
                },
                .padding = {
                    .top = 5.f, 
                    .left = 5.f 
                }
            });
            {
                //NOTE: T.R.S Operation label
                gui_ui_compose_begin(gui, (ui_config_t){
                    .composition = {
                        .styles = UI_STYLE_ONLY_TEXT,
                    },
                    .dim = {
                        .min_width  = 90,
                        .min_height = 30,
                    },
                    .text = transform_member_labels[idx],
                    .color = {
                        .base = COLOR_WHITE
                    },
                    .margin = {
                        .top = 5.f,
                    }
                });
                gui_ui_compose_end(gui);

                ui_config_t value_style = {
                    .composition = {
                        .styles = UI_STYLE_ONLY_TEXT,
                    },
                    .text_align = UI_TEXT_ALIGN_CENTER,
                    .dim = {
                        .min_width  = 40,
                        .min_height = 30,
                    },
                    .text = transform_member_labels[idx],
                    .color = {
                        .base = COLOR_BLACK
                    },
                    .margin = {
                        .top = 5.f,
                    }
                };

                //NOTE: Slider
                gui_ui_compose_begin(gui, (ui_config_t){
                    .composition = {
                        .traits = UI_BEHAVIOR_TRACK_STATE_LOCK_MOUSE_ON_DRAG | UI_BEHAVIOR_HOVERABLE | UI_BEHAVIOR_CLICKABLE
                    },
                    .dim = {
                        .min_width = 50,
                        .min_height = 30,
                    },
                    .color = {
                        .base = COLOR_RED,
                        .highlight = COLOR_LIGHTRED,
                    },
                    .binding = {
                        .ref = (void *)&transform_bindings[idx]->x,
                        .size = sizeof(f32)
                    },
                });
                {
                    //Label
                    memset(tempbuffer, 0, sizeof(tempbuffer));

                    switch(idx)
                    {
                        case OT_TRANSLATION:    snprintf(tempbuffer, sizeof(tempbuffer), "%.2f", transform->position.x);
                        break;
                        case OT_ROTATION:       snprintf(tempbuffer, sizeof(tempbuffer), "%.2f", transform->orientation.x);
                        break;
                        case OT_SCALE:          snprintf(tempbuffer, sizeof(tempbuffer), "%.2f", transform->scale.x);
                        break;
                    }

                    value_style.text = str__from_cstr(tempbuffer, sizeof(tempbuffer));
                    gui_ui_compose_begin(gui, value_style);
                    gui_ui_compose_end(gui);
                gui_ui_compose_end(gui);

                gui_ui_compose_begin(gui, (ui_config_t){
                    .composition = {
                        .traits = UI_BEHAVIOR_TRACK_STATE_LOCK_MOUSE_ON_DRAG | UI_BEHAVIOR_HOVERABLE | UI_BEHAVIOR_CLICKABLE
                    },
                    .color = {
                        .base = COLOR_GREEN,
                        .highlight = COLOR_LIGHTGREEN,
                    },
                    .dim = {
                        .min_width = 50,
                        .min_height = 30,
                    },
                    .binding = {
                        .ref = (void *)&transform_bindings[idx]->y,
                        .size = sizeof(f32)
                    },
                    .margin = {
                        .left = 10.f 
                    }
                });
                    //Label
                    memset(tempbuffer, 0, sizeof(tempbuffer));

                    switch(idx)
                    {
                        case OT_TRANSLATION:    snprintf(tempbuffer, sizeof(tempbuffer), "%.2f", transform->position.y);
                        break;
                        case OT_ROTATION:       snprintf(tempbuffer, sizeof(tempbuffer), "%.2f", transform->orientation.y);
                        break;
                        case OT_SCALE:          snprintf(tempbuffer, sizeof(tempbuffer), "%.2f", transform->scale.y);
                        break;
                    }

                    value_style.text = str__from_cstr(tempbuffer, sizeof(tempbuffer));
                    gui_ui_compose_begin(gui, value_style);
                    gui_ui_compose_end(gui);
                gui_ui_compose_end(gui);

                gui_ui_compose_begin(gui, (ui_config_t){
                    .color = {
                        .base = COLOR_BLUE,
                        .highlight = COLOR_LIGHTBLUE,
                    },
                    .composition = {
                        .traits = UI_BEHAVIOR_TRACK_STATE_LOCK_MOUSE_ON_DRAG | UI_BEHAVIOR_HOVERABLE | UI_BEHAVIOR_CLICKABLE
                    },
                    .dim = {
                        .min_width = 50,
                        .min_height = 30,
                    },
                    .binding = {
                        .ref = (void *)&transform_bindings[idx]->z,
                        .size = sizeof(f32)
                    },
                    .margin = {
                        .left = 10.f 
                    }
                });
                    //Label
                    memset(tempbuffer, 0, sizeof(tempbuffer));

                    switch(idx)
                    {
                        case OT_TRANSLATION:    snprintf(tempbuffer, sizeof(tempbuffer), "%.2f", transform->position.z);
                        break;
                        case OT_ROTATION:       snprintf(tempbuffer, sizeof(tempbuffer), "%.2f", transform->orientation.z);
                        break;
                        case OT_SCALE:          snprintf(tempbuffer, sizeof(tempbuffer), "%.2f", transform->scale.z);
                        break;
                    }

                    value_style.text = str__from_cstr(tempbuffer, sizeof(tempbuffer));
                    gui_ui_compose_begin(gui, value_style);
                    gui_ui_compose_end(gui);

                }
                gui_ui_compose_end(gui); //NOTE: slider for x, y, z

                const u32 reset_id = gui_ui_compose_begin(gui, (ui_config_t) {
                    .composition = {
                        .traits = UI_BEHAVIOR_HOVERABLE | UI_BEHAVIOR_CLICKABLE,
                    },
                    .color = {
                        .base = COLOR_ORANGE,
                        .highlight = COLOR_LIGHTORANGE
                    },
                    .dim = {
                        .min_height = 20,
                        .min_width = 20,
                    },
                    .margin = {
                        .left = 10.f,
                        .top = 10.f
                    },
                });
                    if (gui_ui_isclicked(gui, reset_id)) {
                        if (idx == OT_SCALE)    *transform_bindings[idx] = (vec3f_t){1.f,1.f,1.f};
                        else                    *transform_bindings[idx] = (vec3f_t){0};
                    }
                gui_ui_compose_end(gui);

            }
            gui_ui_compose_end(gui); //NOTE: TRS container

        }
    }
    gui_ui_compose_end(gui);

    transform->orientation = glms_quat_normalize(transform->orientation);
}

INTERNAL void workbench_editor__internal_gizmo_draw_axis(
    const vec3f_t entitypos, 
    const matrix4f_t view, 
    const matrix4f_t proj
) {
    const f32 axis_length = 1000.f;
    rendercommand_instance_line_t gizmo[3] = {
        //NOTE: X axis
        {
            .color       = {1.0f, 0.0f, 0.0f, 1.0f},
            .translation = { entitypos.x, entitypos.y, entitypos.z, 0.f },
            .orientation = {0.0f, 0.0f, 0.0f, 1.0f},
            .scale       = {axis_length, 1.0f, 1.0f, 0.0f},
        },
        //NOTE: Y axis
        {
            .color       = {0.0f, 1.0f, 0.0f, 1.0f},
            .translation = { entitypos.x, entitypos.y, entitypos.z, 0.f },
            .orientation = {0.0f, 0.0f, -0.7071f, 0.7071f},
            .scale       = {axis_length, 1.0f, 1.0f, 0.0f},
        },
        //NOTE: Z axis
        {
            .color       = {0.0f, 0.0f, 1.0f, 1.0f},
            .translation = { entitypos.x, entitypos.y, entitypos.z, 0.f },
            .orientation = {0.0f, -0.7071f, 0.0f, 0.7071f},
            .scale       = {axis_length, 1.0f, 1.0f, 0.0f},
        },
    };

    for (u8 idx = 0; idx < 3; idx++)
    {
        rendercommand_t rendercommand = {
            .draw_mode = RENDER_COMMAND_DRAW_MODE_LINES,
            .instance = {
                .raw_data = {0},
                .size = sizeof(rendercommand_instance_line_t),
            },
            .material = {
                .shader = {
                    .data = assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, global_workbench->primitives.line_shader_id),
                    .uniforms = {
                        .count = 2,
                        .data = {
                            [0] = {
                                .name = str("projection"),
                                .value = proj
                            },
                            [1] = {
                                .name = str("view"),
                                .value = view,
                            }
                        }
                    }
                }
            },
            .mesh = assetmanager_get_gpu_loaded_asset_async(&global_engine->systems.assets, GL_MESH_PRIMITIVE_TYPE_LINE)->meshes.data,
        };

        memcpy(rendercommand.instance.raw_data, &gizmo[idx], sizeof(gizmo[idx]));
        renderqueue_pass_command(&global_engine->systems.renderqueue, rendercommand);
    }
}

INTERNAL void workbench_editor__internal_draw_gizmo_on_entity_selection(void)
{
    if (!global_workbench->editor.current_selected_entity_id) return;

    const ecs_component_transform_t *const transform = ecs_entity_query_components(
        global_ecs, global_workbench->editor.current_selected_entity_id, ECS_CMP_TRANSFORM
    ).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

    if (!transform) return;

    workbench_editor__internal_gizmo_draw_axis(
        transform->position, 
        glcamera_getview(global_workbench->world_camera.handle),
        glms_perspective(radians(45), global_engine->handle.app->window.aspect_ratio, 1.0f, 1000.0f)
    );
}

void workbench_editor_render(void)
{
    workbench_editor__internal_show_entity_info_for_selected_entity();
}

INTERNAL void workbench_editor__internal_update_physics_colliders(void)
{
    if(!global_workbench->editor.current_selected_entity_id) return;

    logging("Updated physics collider for entity(%i)", global_workbench->editor.current_selected_entity_id);

    const u32 entity_id = global_workbench->editor.current_selected_entity_id;
    const ecs_entity_query_t query      = ecs_entity_query_components(global_ecs, entity_id, ECS_CMP_COLLIDER | ECS_CMP_TRANSFORM);
    ecs_component_collider_t *collider  = query.entity_cmp_data[ECS_CMP_COLLIDER_IDX];

    if (!collider) return;

    //NOTE: updating the shape of the collider
    {
        ecs_component_transform_t *const transform = query.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        ASSERT(transform);

        JPH_Shape *const newShape = JPH_Shape_ScaleShape(collider->internal.shape, (JPH_Vec3 *)&transform->scale);
        JPH_BodyInterface_SetShape(global_physics_sys_jolt_instance->bodyinterface, collider->internal.body_id, newShape, false, JPH_Activation_DontActivate);
        JPH_Shape_Destroy(newShape);

        JPH_BodyInterface_SetPositionAndRotation(
            global_physics_sys_jolt_instance->bodyinterface, 
            collider->internal.body_id, 
            (JPH_Vec3 *)&transform->position, 
            &(JPH_Quat) {
                .x = transform->orientation.x,
                .y = transform->orientation.y,
                .z = transform->orientation.z,
                .w = transform->orientation.w,
            },
            JPH_Activation_DontActivate
        );

        collider->internal.position     = transform->position;
        collider->internal.orientation  = transform->orientation;
    }
}

void workbench_editor_update(void)
{
    workbench_editor__internal_check_mouse_closest_entity();
    workbench_editor__internal_draw_gizmo_on_entity_selection();
}

