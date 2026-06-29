#pragma once
#include "poglib/basic/color.h"
#include "poglib/ecs/component/types.h"
#include "poglib/external/joltc/include/joltc.h"
#include "poglib/gfx/model/assimp.h"
#include "poglib/gui.h"
#include "poglib/physics/jolt-wrapper.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/util/asset.h"
#include "poglib/util/assetmanager.h"
#include <poglib/util/workbench/common.h>
#include <poglib/util/workbench/workbench-constants.h>
#include <poglib/ecs.h>

INTERNAL void workbench_editor__internal_ray_to_local(
    const vec3f_t             world_origin,
    const vec3f_t             world_dir,
    const matrix4f_t          inv_world,
    vec3f_t *out_origin,
    vec3f_t *out_dir)
{
    const vec4f_t o4 = glms_mat4_mulv(inv_world, (vec4f_t){ world_origin.x, world_origin.y, world_origin.z, 1.f });
    const vec4f_t d4 = glms_mat4_mulv(inv_world, (vec4f_t){ world_dir.x,   world_dir.y,   world_dir.z,   0.f });
    *out_origin = *(vec3f_t *)&o4;
    *out_dir    = *(vec3f_t *)&d4;
}

INTERNAL bool workbench_editor__internal_lookup_bounds(const u32 entity_id, vec3f_t *out_half_extents, JPH_Shape **out_shape)
{
    for (u32 i = 0; i < global_workbench->editor.selection_bounds_count; i++) {
        if (global_workbench->editor.selection_bounds[i].entity_id == entity_id) {
            *out_half_extents = global_workbench->editor.selection_bounds[i].half_extents;
            *out_shape        = global_workbench->editor.selection_bounds[i].shape;
            return true;
        }
    }
    return false;
}

INTERNAL void workbench_editor__internal_cache_bounds(const u32 entity_id, const vec3f_t half_extents, JPH_Shape *shape)
{
    if (global_workbench->editor.selection_bounds_count < EDITOR_SELECTION_BOUNDS_MAX) {
        u32 idx = global_workbench->editor.selection_bounds_count++;
        global_workbench->editor.selection_bounds[idx].entity_id    = entity_id;
        global_workbench->editor.selection_bounds[idx].half_extents = half_extents;
        global_workbench->editor.selection_bounds[idx].shape        = shape;
    }
}

INTERNAL vec3f_t workbench_editor__internal_camera_aabb_halfextents(void)
{
    f32 min[3] = { INFINITY,  INFINITY,  INFINITY };
    f32 max[3] = { -INFINITY, -INFINITY, -INFINITY };

    const u32 vert_count = ARRAY_LEN(CAMERA_VERTICES) / 3;
    for (u32 i = 0; i < vert_count; i++) {
        for (u32 j = 0; j < 3; j++) {
            const f32 v = CAMERA_VERTICES[i * 3 + j];
            if (v < min[j]) min[j] = v;
            if (v > max[j]) max[j] = v;
        }
    }

    return (vec3f_t){
        (max[0] - min[0]) * 0.5f,
        (max[1] - min[1]) * 0.5f,
        (max[2] - min[2]) * 0.5f,
    };
}

INTERNAL vec3f_t workbench_editor__internal_model_compute_aabb_halfextents(const u32 model_asset_id)
{
    const glmodel_t *model = assetmanager_get_assetresource(
        &global_engine->systems.assets, ASSET_TYPE_MODEL, model_asset_id);

    f32 min[3] = { INFINITY,  INFINITY,  INFINITY };
    f32 max[3] = { -INFINITY, -INFINITY, -INFINITY };

    if (!model) goto fallback;

    list_iterator(&model->meshes, mesh_iter) {
        const glmesh_t *mesh = mesh_iter;
        slot_iterator(&mesh->vtx, vtx_iter) {
            const glvertex3d_t *v = vtx_iter;
            for (u32 j = 0; j < 3; j++) {
                const f32 coord = ((f32 *)&v->pos)[j];
                if (coord < min[j]) min[j] = coord;
                if (coord > max[j]) max[j] = coord;
            }
        }
    }

    if (min[0] > max[0]) goto fallback;

    return (vec3f_t){
        (max[0] - min[0]) * 0.5f,
        (max[1] - min[1]) * 0.5f,
        (max[2] - min[2]) * 0.5f,
    };

fallback:
    return (vec3f_t){ 1.0f, 1.0f, 1.0f };
}

INTERNAL void workbench_editor__internal_get_entity_bounds(const u32 entity_id, vec3f_t *out_he, JPH_Shape **out_shape)
{
    if (workbench_editor__internal_lookup_bounds(entity_id, out_he, out_shape))
        return;

    vec3f_t he;

    // Model takes priority (most accurate bounds)
    const ecs_entity_query_t q_model = ecs_entity_query_components(global_ecs, entity_id, ECS_CMP_MODEL | ECS_CMP_TRANSFORM);
    if (q_model.entity_cmp_data[ECS_CMP_MODEL_IDX]) {
        const ecs_component_model_t *m = q_model.entity_cmp_data[ECS_CMP_MODEL_IDX];
        he = workbench_editor__internal_model_compute_aabb_halfextents(m->asset_id);
    }
    // Camera entity has known vertex data
    else if (entity_id == global_workbench->world_camera.entity_id) {
        he = workbench_editor__internal_camera_aabb_halfextents();
    }
    // Default: unit box, scale applied via world matrix
    else {
        he = (vec3f_t){ 1.0f, 1.0f, 1.0f };
    }

    JPH_Shape *shape = (JPH_Shape *)JPH_BoxShape_Create((JPH_Vec3 *)&he, 0.f);

    logging("[EDITOR] cached shape for entity=%u he=(%.2f %.2f %.2f) shape=%p",
        entity_id, he.x, he.y, he.z, shape);

    workbench_editor__internal_cache_bounds(entity_id, he, shape);
    *out_he    = he;
    *out_shape = shape;
}

INTERNAL versors workbench_editor__internal_quat_from_x_to_dir(const vec3f_t dir)
{
    const vec3f_t x_axis = { 1.0f, 0.0f, 0.0f };
    const vec3f_t d      = glms_vec3_normalize(dir);
    const f32     dot    = glms_vec3_dot(x_axis, d);

    if (dot > 0.9999f)
        return (versors){ 0.0f, 0.0f, 0.0f, 1.0f };

    if (dot < -0.9999f)
        return glms_quatv(GLM_PIf, (vec3f_t){ 0.0f, 1.0f, 0.0f });

    const vec3f_t axis = glms_vec3_normalize(glms_cross(x_axis, d));
    return glms_quatv(acosf(dot), axis);
}

INTERNAL void workbench_editor__internal_check_mouse_closest_entity(void)
{
    vec2f_t      ndc = window_mouse_get_norm_position(global_window);
    glcamera_t  *cam = global_workbench->world_camera.handle;

    vec3f_t dir = {0};
    {
        const matrix4f_t view   = glcamera_getview(cam);
        const matrix4f_t proj   = glms_perspective(radians(45), global_engine->handle.app->window.aspect_ratio, 0.1f, 1000.0f);
        const matrix4f_t inv_pv = glms_mat4_inv(glms_mat4_mul(proj, view));
        const vec4f_t    nearp  = { ndc.x, ndc.y, -1.0f, 1.0f };
        const vec4f_t    farp   = { ndc.x, ndc.y,  1.0f, 1.0f };

        vec4f_t near_w   = glms_mat4_mulv(inv_pv, nearp);
        vec4f_t far_w    = glms_mat4_mulv(inv_pv, farp);
        near_w           = glms_vec4_scale(near_w, 1.0f / near_w.w);
        far_w            = glms_vec4_scale(far_w,  1.0f / far_w.w);

        dir              = glms_vec3_normalize(glms_vec3_sub(*(vec3f_t *)&far_w, *(vec3f_t *)&near_w));
    }

    const vec3f_t ray_origin = cam->position;
    u32 picked        = 0;
    f32 closest_dist  = INFINITY;

    logging("[EDITOR] ray_origin=(%.2f %.2f %.2f) dir=(%.2f %.2f %.2f)",
        ray_origin.x, ray_origin.y, ray_origin.z,
        dir.x, dir.y, dir.z);

    // ---- Pass 1: Physics raycast for entities with colliders ----
    {
        const JPH_NarrowPhaseQuery *npq = JPH_PhysicsSystem_GetNarrowPhaseQuery(
            global_physics_sys_jolt_instance->physics_system);
        JPH_RayCastResult hit = {0};
        if (JPH_NarrowPhaseQuery_CastRay(npq, (JPH_Vec3 *)&ray_origin, (JPH_Vec3 *)&dir,
                &hit, NULL, NULL, NULL)) {
            if (hit.fraction > 0.f) {
                const u64 ud = JPH_BodyInterface_GetUserData(
                    global_physics_sys_jolt_instance->bodyinterface, hit.bodyID);
                if (ud) {
                    picked       = ((ecs_collider_jolt_userdata_t *)ud)->entity_id;
                    closest_dist = hit.fraction;
                    logging("[EDITOR] Pass1 HIT entity=%u dist=%.2f", picked, closest_dist);
                }
            }
        }
    }

    // ---- Pass 2: Jolt shape raycast for entities without physics colliders ----
    slot_iterator(&global_ecs->managers.entitymanager.entities, iter)
    {
        if (!slot_iterator_index) continue;

        const ecs_entity_t        *const e = iter;
        const ecs_entity_query_t  q       = ecs_entity_query_components(global_ecs, e->id,
                                                   ECS_CMP_TRANSFORM | ECS_CMP_COLLIDER);
        const ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        if (!t) continue;

        // Skip entities that already have physics colliders (handled by Pass 1)
        if (q.entity_cmp_data[ECS_CMP_COLLIDER_IDX]) continue;

        vec3f_t local_he;
        JPH_Shape *shape;
        workbench_editor__internal_get_entity_bounds(e->id, &local_he, &shape);
        if (!shape) { logging("[EDITOR] Pass2 SKIP entity=%u (no shape)", e->id); continue; }

        const matrix4f_t world_mat = glms_mat4_mul(
            glms_mat4_mul(glms_translate_make(t->position),
                          glms_quat_mat4(t->orientation)),
            glms_scale_make(t->scale));
        const matrix4f_t inv_world = glms_mat4_inv(world_mat);

        // JPH_Shape_CastRay treats direction as the ray segment end-point
        // (fraction ∈ [0,1] along origin→origin+direction). The local direction
        // |M_inv * world_dir| can be very short when entity has large scale,
        // so scale it to span the entire scene.
        const vec3f_t world_dir_long = glms_vec3_scale(dir, 10000.f);

        vec3f_t local_origin, local_dir_long;
        workbench_editor__internal_ray_to_local(ray_origin, world_dir_long, inv_world, &local_origin, &local_dir_long);

        JPH_RayCastResult shape_hit;
        if (JPH_Shape_CastRay(shape, (JPH_Vec3 *)&local_origin, (JPH_Vec3 *)&local_dir_long, &shape_hit)) {
            if (shape_hit.fraction > 0.f) {
                const vec3f_t local_hit = glms_vec3_add(local_origin, glms_vec3_scale(local_dir_long, shape_hit.fraction));
                const vec4f_t world_hit4 = glms_mat4_mulv(world_mat,
                    (vec4f_t){ local_hit.x, local_hit.y, local_hit.z, 1.f });
                const vec3f_t world_hit = *(vec3f_t *)&world_hit4;
                const f32 dist = glms_vec3_dot(glms_vec3_sub(world_hit, ray_origin), dir);

                logging("[EDITOR] Pass2 HIT entity=%u dist=%.2f (d_min=%.2f)",
                    e->id, dist, closest_dist);

                if (dist > 0.f && dist < closest_dist) {
                    closest_dist = dist;
                    picked       = e->id;
                }
            }
        }
    }

    logging("[EDITOR] closest=entity=%u dist=%.2f", picked, closest_dist);
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

INTERNAL void workbench_editor__internal_submit_line_command(
    const vec3f_t start,
    const vec3f_t end,
    const vec4f_t color,
    const matrix4f_t view,
    const matrix4f_t proj)
{
    const vec3f_t edge_dir = glms_vec3_sub(end, start);
    const f32     length   = glms_vec3_norm(edge_dir);
    if (length < 1e-6f) return;

    const versors rot = workbench_editor__internal_quat_from_x_to_dir(edge_dir);

    rendercommand_instance_line_t instance = {
        .color       = color,
        .translation = { start.x, start.y, start.z, 0.f },
        .orientation = { rot.x, rot.y, rot.z, rot.w },
        .scale       = { length, 1.0f, 1.0f, 0.0f },
    };

    rendercommand_t rc = {
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
                        [0] = { .name = str("projection"), .value = proj },
                        [1] = { .name = str("view"),       .value = view },
                    }
                }
            }
        },
        .mesh = assetmanager_get_gpu_loaded_asset_async(&global_engine->systems.assets, GL_MESH_PRIMITIVE_TYPE_LINE)->meshes.data,
    };

    memcpy(rc.instance.raw_data, &instance, sizeof(instance));
    renderqueue_pass_command(&global_engine->systems.renderqueue, rc);
}

INTERNAL void workbench_editor__internal_draw_oob_on_entity_selection(void)
{
    if (!global_workbench->editor.current_selected_entity_id) return;

    const u32 entity_id = global_workbench->editor.current_selected_entity_id;

    const ecs_entity_query_t q = ecs_entity_query_components(global_ecs, entity_id, ECS_CMP_TRANSFORM);
    const ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
    if (!t) return;

    JPH_Shape *ignore_shape;
    vec3f_t he;
    workbench_editor__internal_get_entity_bounds(entity_id, &he, &ignore_shape);

    const matrix4f_t view = glcamera_getview(global_workbench->world_camera.handle);
    const matrix4f_t proj = glms_perspective(radians(45), global_engine->handle.app->window.aspect_ratio, 1.0f, 1000.0f);

    const matrix4f_t world_mat = glms_mat4_mul(
        glms_mat4_mul(glms_translate_make(t->position),
                      glms_quat_mat4(t->orientation)),
        glms_scale_make(t->scale));

    const f32 hx = he.x;
    const f32 hy = he.y;
    const f32 hz = he.z;

    const vec3f_t local_corners[8] = {
        { -hx, -hy, -hz }, { +hx, -hy, -hz },
        { +hx, +hy, -hz }, { -hx, +hy, -hz },
        { -hx, -hy, +hz }, { +hx, -hy, +hz },
        { +hx, +hy, +hz }, { -hx, +hy, +hz },
    };

    vec3f_t world_corners[8];
    for (u32 i = 0; i < 8; i++) {
        const vec4f_t w4 = glms_mat4_mulv(world_mat,
            (vec4f_t){ local_corners[i].x, local_corners[i].y, local_corners[i].z, 1.f });
        world_corners[i] = *(vec3f_t *)&w4;
    }

    const u32 edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7},
    };

    const vec4f_t oob_color = COLOR_ORANGE;

    for (u32 i = 0; i < 12; i++) {
        workbench_editor__internal_submit_line_command(
            world_corners[edges[i][0]],
            world_corners[edges[i][1]],
            oob_color, view, proj);
    }
}

INTERNAL void workbench_editor__internal_gizmo_draw_axis(
    const vec3f_t entitypos, 
    const matrix4f_t view, 
    const matrix4f_t proj
) {
    const f32 axis_length = 1000.f;
    rendercommand_instance_line_t gizmo[3] = {
        {
            .color       = {1.0f, 0.0f, 0.0f, 1.0f},
            .translation = { entitypos.x, entitypos.y, entitypos.z, 0.f },
            .orientation = {0.0f, 0.0f, 0.0f, 1.0f},
            .scale       = {axis_length, 1.0f, 1.0f, 0.0f},
        },
        {
            .color       = {0.0f, 1.0f, 0.0f, 1.0f},
            .translation = { entitypos.x, entitypos.y, entitypos.z, 0.f },
            .orientation = {0.0f, 0.0f, -0.7071f, 0.7071f},
            .scale       = {axis_length, 1.0f, 1.0f, 0.0f},
        },
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
    workbench_editor__internal_draw_oob_on_entity_selection();
}

