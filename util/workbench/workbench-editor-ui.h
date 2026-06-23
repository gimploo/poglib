#pragma once
#include "poglib/poggen.h"
#include "poglib/ecs.h"
#include "poglib/ecs/common.h"
#include <poglib/gui.h>

void workbench_editor_compose(gui_t *gui, workbench_t *wb);

#ifndef IGNORE_WORKBENCH_EDITOR_IMPLEMENTATION

enum {
    WB_EDITOR_SLIDER_X,
    WB_EDITOR_SLIDER_Y,
    WB_EDITOR_SLIDER_Z,
    WB_EDITOR_INPUT_NAME,
    WB_EDITOR_TOGGLE_VISIBLE,
    WB_EDITOR_DELETE,
    WB_EDITOR_ADD_ENTITY,
};

f32 workbench_editor_closest_point_on_ray(const vec3f_t ray_origin, const vec3f_t ray_dir, const vec3f_t point)
{
    vec3f_t to_point = glms_vec3_sub(point, ray_origin);
    f32 t = glms_vec3_dot(to_point, ray_dir);
    if (t < 0.f) t = 0.f;
    vec3f_t closest = glms_vec3_add(ray_origin, glms_vec3_scale(ray_dir, t));
    return glms_vec3_distance(closest, point);
}

void workbench_editor_pick_entity(void)
{
    vec2f_t ndc = window_mouse_get_norm_position(global_window);
    glcamera_t *cam = ecs_get_active_camera(global_ecs);
    vec3f_t dir = {0};
    {
        matrix4f_t view = glcamera_getview(cam);
        matrix4f_t proj = glms_perspective(radians(45), global_engine->handle.app->window.aspect_ratio, 0.1f, 1000.0f);
        matrix4f_t inv_pv = glms_mat4_inv(glms_mat4_mul(proj, view));
        vec4f_t near = { ndc.x, ndc.y, -1.0f, 1.0f };
        vec4f_t far  = { ndc.x, ndc.y,  1.0f, 1.0f };
        vec4f_t near_w = glms_mat4_mulv(inv_pv, near);
        vec4f_t far_w  = glms_mat4_mulv(inv_pv, far);
        near_w = glms_vec4_scale(near_w, 1.0f / near_w.w);
        far_w  = glms_vec4_scale(far_w,  1.0f / far_w.w);
        dir = glms_vec3_normalize(glms_vec3_sub(*(vec3f_t *)&far_w, *(vec3f_t *)&near_w));
    }

    u32 picked = 0;
    f32 closest_dist = 5.f;

    JPH_RayCastResult hit = physics_sys_jolt_raycast(cam->position, dir);
    if (hit.bodyID) {
        picked = ecs_entity_get_id_by_body_id(hit.bodyID);
    }

    if (!picked) {
        ecs_entity_foreach(eid) {
            const ecs_entity_query_t q = ecs_entity_query_components(global_ecs, eid, ECS_CMP_TRANSFORM);
            ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
            if (!t) continue;
            f32 d = workbench_editor_closest_point_on_ray(cam->position, dir, t->position);
            if (d < closest_dist) {
                closest_dist = d;
                picked = eid;
            }
        }
    }

    global_workbench->selected_entity_id = picked;
}

void workbench_editor_compose(gui_t *gui, workbench_t *wb)
{
    if (!wb->selected_entity_id) return;

    const ecs_entity_query_t view = ecs_entity_query_components(
        global_ecs, wb->selected_entity_id, ECS_CMP_TRANSFORM);
    ecs_component_transform_t *t = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
    if (!t) return;

    gui_ui_compose_begin(gui, (ui_config_t){
        .composition = { .styles = UI_STYLE_ROUNDED_CORNERS },
        .color = { .base = (vec4f_t){0.15f, 0.15f, 0.15f, 0.92f} },
        .dim = { .min_height = 200, .mid_width = 240 },
        .margin = { .left = (u32)(global_window->width - 260), .top = 60 },
        .padding = { 8, 8, 8, 8 }
    });

    char tmp[64];

    snprintf(tmp, sizeof(tmp), "Entity %u", wb->selected_entity_id);
    gui_ui_compose_begin(gui, (ui_config_t){
        .dim = { .min_height = 20, .mid_width = 180 },
        .color = { .base = COLOR_WHITE },
        .label = str__from_cstr(tmp, sizeof(tmp)),
        .margin = { 0, 0, 4, 0 }
    });
    gui_ui_compose_end(gui);

    snprintf(tmp, sizeof(tmp), "%.2f", t->position.x);
    gui_ui_compose_begin(gui, (ui_config_t){
        .dim = { .min_height = 16, .mid_width = 30 },
        .color = { .base = (vec4f_t){0.8f, 0.2f, 0.2f, 1.0f} },
        .label = str("X"),
        .margin = { 0, 0, 0, 0 }
    });
    gui_ui_compose_end(gui);
    f32 px = gui_slider_f32(gui, WB_EDITOR_SLIDER_X, t->position.x, -100.f, 100.f);

    snprintf(tmp, sizeof(tmp), "%.2f", t->position.y);
    gui_ui_compose_begin(gui, (ui_config_t){
        .dim = { .min_height = 16, .mid_width = 30 },
        .color = { .base = (vec4f_t){0.2f, 0.8f, 0.2f, 1.0f} },
        .label = str("Y"),
        .margin = { 0, 0, 0, 0 }
    });
    gui_ui_compose_end(gui);
    f32 py = gui_slider_f32(gui, WB_EDITOR_SLIDER_Y, t->position.y, -100.f, 100.f);

    snprintf(tmp, sizeof(tmp), "%.2f", t->position.z);
    gui_ui_compose_begin(gui, (ui_config_t){
        .dim = { .min_height = 16, .mid_width = 30 },
        .color = { .base = (vec4f_t){0.2f, 0.2f, 0.8f, 1.0f} },
        .label = str("Z"),
        .margin = { 0, 0, 0, 0 }
    });
    gui_ui_compose_end(gui);
    f32 pz = gui_slider_f32(gui, WB_EDITOR_SLIDER_Z, t->position.z, -100.f, 100.f);

    if (px != t->position.x || py != t->position.y || pz != t->position.z) {
        ecs_set_entity_transform(global_ecs, wb->selected_entity_id,
            (vec3f_t){ px, py, pz }, t->orientation, t->scale);
    }

    gui_ui_compose_begin(gui, (ui_config_t){
        .id = WB_EDITOR_DELETE,
        .composition = { .traits = UI_BEHAVIOR_CLICKABLE | UI_BEHAVIOR_HOVERABLE },
        .dim = { .min_height = 24, .mid_width = 80 },
        .color = { .base = (vec4f_t){0.6f, 0.1f, 0.1f, 1.0f}, .highlight = (vec4f_t){0.8f, 0.2f, 0.2f, 1.0f} },
        .margin = { 0, 0, 4, 0 },
        .padding = { 4, 4, 4, 4 }
    });
    gui_ui_compose_begin(gui, (ui_config_t){
        .dim = { .min_height = 18, .mid_width = 60 },
        .color = { .base = COLOR_WHITE },
        .label = str("Delete"),
        .margin = { 0 }
    });
    gui_ui_compose_end(gui);
    gui_ui_compose_end(gui);

    if (gui_ui_ishovered(gui, WB_EDITOR_DELETE) && window_mouse_button_just_pressed(global_window, SDL_MOUSEBUTTON_LEFT)) {
        ecs_entity_remove(global_ecs, wb->selected_entity_id);
        wb->selected_entity_id = 0;
    }

    gui_ui_compose_end(gui);
}

void workbench_editor_add_entity_button(gui_t *gui)
{
    gui_ui_compose_begin(gui, (ui_config_t){
        .id = WB_EDITOR_ADD_ENTITY,
        .composition = { .traits = UI_BEHAVIOR_CLICKABLE | UI_BEHAVIOR_HOVERABLE },
        .dim = { .min_height = 30, .mid_width = 120 },
        .color = { .base = (vec4f_t){0.1f, 0.5f, 0.1f, 1.0f}, .highlight = (vec4f_t){0.2f, 0.7f, 0.2f, 1.0f} },
        .margin = { .left = 5, .top = 10 },
        .padding = { 5, 5, 5, 5 }
    });
    gui_ui_compose_begin(gui, (ui_config_t){
        .dim = { .min_height = 20, .mid_width = 80 },
        .color = { .base = COLOR_WHITE },
        .label = str("Add Entity"),
        .margin = { 0 }
    });
    gui_ui_compose_end(gui);
    gui_ui_compose_end(gui);

    if (gui_ui_ishovered(gui, WB_EDITOR_ADD_ENTITY) && window_mouse_button_just_pressed(global_window, SDL_MOUSEBUTTON_LEFT)) {
        ecs_entity_add(global_ecs, (ecs_componentbundle_t){
            .signature = ECS_CMP_TRANSFORM | ECS_CMP_MODEL | ECS_CMP_MATERIAL,
            .component = {
                [ECS_CMP_TRANSFORM_IDX].transform = {
                    .position = {0, 0, 0},
                    .orientation = GLMS_QUAT_IDENTITY_INIT,
                    .scale = {1, 1, 1}
                },
                [ECS_CMP_MODEL_IDX].model = {0},
                [ECS_CMP_MATERIAL_IDX].material = {0}
            }
        });
    }
}

#endif
