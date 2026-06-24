#pragma once
#include "poglib/poggen.h"
#include "poglib/ecs.h"
#include "poglib/ecs/common.h"
#include "./common.h"


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

/* Slider interaction state — persists across frames */
static i32 wb_editor_active_slider = -1;
static f32 wb_editor_slider_bounds[3][4]; /* [slider] = {x, y, w, h} from last frame */

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

    ecs_entity_foreach(ent_iter) {
        ecs_entity_t *e = ent_iter;
        ecs_entity_query_t q = ecs_entity_query_components(global_ecs, e->id, ECS_CMP_TRANSFORM);
        ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        if (!t) continue;
        f32 d = workbench_editor_closest_point_on_ray(cam->position, dir, t->position);
        if (d < closest_dist) {
            closest_dist = d;
            picked = e->id;
        }
    }

    global_workbench->selected_entity_id = picked;
    wb_editor_active_slider = -1;
}

static void wb_editor_slider_click_cb(Clay_ElementId id, Clay_PointerData ptr, intptr_t user)
{
    (void)id;
    if (ptr.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
        wb_editor_active_slider = (i32)user;
}

static void wb_editor_handle_delete_click(workbench_t *wb)
{
    Clay_ElementId del_id = Clay_GetElementId(CLAY_STRING("DeleteBtn"));
    if (Clay_PointerOver(del_id)) {
        if (window_mouse_button_just_pressed(global_window, SDL_BUTTON_LEFT)) {
            ecs_entity_remove(global_ecs, wb->selected_entity_id);
            wb->selected_entity_id = 0;
            wb_editor_active_slider = -1;
        }
    }
}

static f32 wb_editor_slider(clay_poglib_renderer_t *r, workbench_t *wb,
                             i32 slider_idx, f32 val, f32 min, f32 max,
                             vec4f_t color, const char *label)
{
    (void)r;
    (void)wb;
    f32 panel_w = 260.0f;
    f32 label_w = 18.0f;
    f32 track_w = panel_w - 24.0f - label_w - 4.0f;
    f32 t = (val - min) / (max - min);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    f32 thumb_x = t * (track_w - 8.0f);

    int n = snprintf(wb->clay.text_buf + wb->clay.text_offset,
                     sizeof(wb->clay.text_buf) - (u32)wb->clay.text_offset, "%s", label);
    { Clay_String s = { .length = n, .chars = wb->clay.text_buf + wb->clay.text_offset };
    CLAY_TEXT(s, CLAY_TEXT_CONFIG({
        .fontId = 0, .fontSize = 13,
        .textColor = { (u8)(color.x * 255), (u8)(color.y * 255),
                       (u8)(color.z * 255), 255 }
    })); }
    wb->clay.text_offset += n + 1;

    char idbuf[32];
    n = snprintf(idbuf, sizeof(idbuf), "SliderTrack%d", slider_idx);
    Clay_String id_ss = { .length = n, .chars = idbuf };
    Clay_ElementId track_id = Clay_GetElementId(id_ss);
    CLAY({
        .id = track_id,
        .layout = { .sizing = { .width = CLAY_SIZING_FIXED(track_w), .height = CLAY_SIZING_FIXED(16) },
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } },
        .backgroundColor = { 50, 50, 55, 255 },
        .cornerRadius = CLAY_CORNER_RADIUS(3)
    }) {
        Clay_OnHover(wb_editor_slider_click_cb, (intptr_t)slider_idx);
        if (t > 0.005f) {
            CLAY({
                .layout = { .sizing = { .width = CLAY_SIZING_FIXED(thumb_x + 4.0f), .height = CLAY_SIZING_FIXED(16) } },
                .backgroundColor = { (u8)(color.x * 255), (u8)(color.y * 255), (u8)(color.z * 255), 180 },
                .cornerRadius = CLAY_CORNER_RADIUS(3)
            }) {}
        }
    }

    if (wb_editor_active_slider == slider_idx) {
        f32 *b = wb_editor_slider_bounds[slider_idx];
        vec2i_t m = window_mouse_get_position(global_window);
        f32 local_x = (f32)m.x - b[0];
        f32 raw_t = local_x / b[2];
        if (raw_t < 0.0f) raw_t = 0.0f;
        if (raw_t > 1.0f) raw_t = 1.0f;
        val = min + raw_t * (max - min);

        if (!window_mouse_button_is_held(global_window, SDL_BUTTON_LEFT))
            wb_editor_active_slider = -1;
    }

    return val;
}

void workbench_editor_compose(clay_poglib_renderer_t *r, workbench_t *wb)
{
    if (!wb->selected_entity_id) return;

    const ecs_entity_query_t view = ecs_entity_query_components(
        global_ecs, wb->selected_entity_id, ECS_CMP_TRANSFORM);
    ecs_component_transform_t *t = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
    if (!t) return;

    f32 ww = (f32)global_window->width;
    f32 panel_w = 260.0f;
    f32 panel_x = ww - panel_w - 10.0f;
    f32 panel_y = 310.0f;

    CLAY({
        .id = CLAY_ID("EntityEditorPanel"),
        .layout = {
            .sizing = { .width = CLAY_SIZING_FIXED(panel_w), .height = CLAY_SIZING_FIT() },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = { 12, 12, 12, 12 },
            .childGap = 6
        },
        .floating = {
            .attachTo = CLAY_ATTACH_TO_ROOT,
            .offset = { panel_x, panel_y },
            .zIndex = 101
        },
        .backgroundColor = { 30, 30, 35, 230 },
        .cornerRadius = CLAY_CORNER_RADIUS(8)
    }) {
        int n = snprintf(wb->clay.text_buf + wb->clay.text_offset,
                         sizeof(wb->clay.text_buf) - (u32)wb->clay.text_offset,
                         "Edit Entity %u", wb->selected_entity_id);
        { Clay_String s = { .length = n, .chars = wb->clay.text_buf + wb->clay.text_offset };
        CLAY_TEXT(s, CLAY_TEXT_CONFIG({
            .fontId = 0, .fontSize = 15, .textColor = { 220, 220, 220, 255 }
        })); }
        wb->clay.text_offset += n + 1;

        CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(1) } },
               .backgroundColor = { 80, 80, 90, 255 } }) {}

        CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 4, .padding = { 0, 0, 0, 0 } } }) {
            f32 px = wb_editor_slider(r, wb, 0, t->position.x, -100.f, 100.f,
                (vec4f_t){0.8f, 0.2f, 0.2f, 1.0f}, "X");
            if (px != t->position.x) {
                t->position.x = px;
                ecs_set_entity_transform(global_ecs, wb->selected_entity_id, t->position, t->orientation, t->scale);
            }
        }
        CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 4, .padding = { 0, 0, 0, 0 } } }) {
            f32 py = wb_editor_slider(r, wb, 1, t->position.y, -100.f, 100.f,
                (vec4f_t){0.2f, 0.8f, 0.2f, 1.0f}, "Y");
            if (py != t->position.y) {
                t->position.y = py;
                ecs_set_entity_transform(global_ecs, wb->selected_entity_id, t->position, t->orientation, t->scale);
            }
        }
        CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 4, .padding = { 0, 0, 0, 0 } } }) {
            f32 pz = wb_editor_slider(r, wb, 2, t->position.z, -100.f, 100.f,
                (vec4f_t){0.2f, 0.2f, 0.8f, 1.0f}, "Z");
            if (pz != t->position.z) {
                t->position.z = pz;
                ecs_set_entity_transform(global_ecs, wb->selected_entity_id, t->position, t->orientation, t->scale);
            }
        }

        CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(1) } },
               .backgroundColor = { 80, 80, 90, 255 } }) {}

        CLAY({
            .id = CLAY_ID("DeleteBtn"),
            .layout = { .sizing = { .width = CLAY_SIZING_FIXED(80), .height = CLAY_SIZING_FIXED(26) },
                        .padding = { 6, 6, 6, 6 } },
            .backgroundColor = { 180, 40, 40, 255 },
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }) {
            { Clay_String s = { .length = 6, .chars = "Delete" };
            CLAY_TEXT(s, CLAY_TEXT_CONFIG({
                .fontId = 0, .fontSize = 13, .textColor = { 255, 255, 255, 255 }
            })); }
        }
    }

    wb_editor_handle_delete_click(wb);
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

    const u32 wb_click = commandqueue_get_commands_as_bitmask(&global_engine->systems.commandqueue);
    if (gui_ui_ishovered(gui, WB_EDITOR_ADD_ENTITY) && (wb_click & (1 << WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_JUST_CLICKED))) {
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
