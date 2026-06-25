#pragma once
#include "./common.h"
#include "poglib/ecs.h"
#include "poglib/gui.h"


typedef struct {

    u32 highlighted_entity_id;

} workbench_editor_t;



f32 workbench_editor__internal_closest_point_on_ray(const vec3f_t ray_origin, const vec3f_t ray_dir, const vec3f_t targetpoint)
{
    const vec3f_t to_point = glms_vec3_sub(targetpoint, ray_origin);
    f32 t = glms_vec3_dot(to_point, ray_dir);
    if (t < 0.f) t = 0.f;
    vec3f_t closest = glms_vec3_add(ray_origin, glms_vec3_scale(ray_dir, t));
    return glms_vec3_distance(closest, targetpoint);
}

void workbench_editor__internal_check_mouse_closest_entity(void)
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

void workbench_editor__internal_show_entity_info_for_selected_entity(void)
{
    if (!global_workbench->editor.selected_entity_id) return;

    gui_t *const gui = &global_workbench->gui.handle;

    gui_ui_compose_begin(gui, (ui_config_t) {
            .dim = {
                .min_width = 200,
                .min_height = 200,
            },
            .margin = {
                .top = 20.f,
                .left = 20.f
            },
            .color = {
                .base = COLOR_DARK_GRAY
            }
    });
        gui_ui_compose_begin(gui, (ui_config_t) {
                .id = 1,
                .text = str("show transform"),
                .composition = {
                    .styles = UI_STYLE_ONLY_TEXT,
                    .traits = UI_BEHAVIOR_CLICKABLE 
                },
                .dim = {
                    .min_width = 20,
                    .min_height = 15,
                },
                .margin = {
                    .top = 20.f,
                    .left = 20.f
                },
                .color = {
                    .base = COLOR_WHITE
                }
        });
        gui_ui_compose_end(gui);
        gui_ui_compose_begin(gui, (ui_config_t) {
                .id = 2,
                .text = str("rotation"),
                .composition = {
                    .traits = UI_BEHAVIOR_CLICKABLE,
                    .styles = UI_STYLE_ONLY_TEXT
                },
                .dim = {
                    .min_width = 15,
                    .min_height = 10,
                },
                .margin = {
                    .top = 20.f,
                    .left = 20.f
                },
                .color = {
                    .base = COLOR_WHITE
                }
        });
        gui_ui_compose_end(gui);
        gui_ui_compose_begin(gui, (ui_config_t) {
                .id = 3,
                .text = str("scale"),
                .composition = {
                    .styles = UI_STYLE_ONLY_TEXT,
                    .traits = UI_BEHAVIOR_CLICKABLE 
                },
                .dim = {
                    .min_width = 15,
                    .min_height = 10,
                },
                .margin = {
                    .top = 20.f,
                    .left = 20.f
                },
                .color = {
                    .base = COLOR_WHITE
                }
        });
        gui_ui_compose_end(gui);
    gui_ui_compose_end(gui);
}

void workbench_editor_render(void)
{
    workbench_editor__internal_show_entity_info_for_selected_entity();
}

void workbench_editor_update(void)
{
    workbench_editor__internal_check_mouse_closest_entity();
}

