#pragma once

static void workbench_editor_entity_details_compose(workbench_t *wb)
{
    if (!wb->selected_entity_id) return;

    u32 ecs_id = wb->selected_entity_id;

    u32 entity_sig = 0;
    slot_iterator(&global_ecs->managers.entitymanager.entities, ent_iter) {
        ecs_entity_t *e = ent_iter;
        if (e->id == ecs_id) {
            entity_sig = e->component_signature;
            break;
        }
    }
    if (!entity_sig) return;

    ecs_entity_query_t view = ecs_entity_query_components(global_ecs, ecs_id, entity_sig);
    ecs_component_transform_t *t = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

    f32 ww = (f32)global_window->width;
    f32 panel_w = 280.0f;
    f32 panel_x = ww - panel_w - 10.0f;

    CLAY({
        .id = CLAY_ID("EntityDetailsPanel"),
        .layout = {
            .sizing = { .width = CLAY_SIZING_FIXED(panel_w), .height = CLAY_SIZING_FIT() },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = { 12, 12, 12, 12 },
            .childGap = 8
        },
        .floating = {
            .attachTo = CLAY_ATTACH_TO_ROOT,
            .offset = { panel_x, 60.0f },
            .zIndex = 100
        },
        .backgroundColor = { 25, 25, 30, 230 },
        .cornerRadius = CLAY_CORNER_RADIUS(8)
    }) {
        char buf[128];
        int n;

        n = snprintf(buf, sizeof(buf), "Entity %u", ecs_id);
        { Clay_String s = { .length = n, .chars = buf };
        CLAY_TEXT(s, CLAY_TEXT_CONFIG({
            .fontId = 0, .fontSize = 18, .textColor = { 220, 220, 220, 255 }
        })); }

        if (t) {
            CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(1) } },
                   .backgroundColor = { 80, 80, 90, 255 } }) {}

            n = snprintf(buf, sizeof(buf), "Position:  %.2f, %.2f, %.2f",
                         t->position.x, t->position.y, t->position.z);
            { Clay_String s = { .length = n, .chars = buf };
            CLAY_TEXT(s, CLAY_TEXT_CONFIG({
                .fontId = 0, .fontSize = 14, .textColor = { 180, 180, 190, 255 }
            })); }

            n = snprintf(buf, sizeof(buf), "Scale:     %.2f, %.2f, %.2f",
                         t->scale.x, t->scale.y, t->scale.z);
            { Clay_String s = { .length = n, .chars = buf };
            CLAY_TEXT(s, CLAY_TEXT_CONFIG({
                .fontId = 0, .fontSize = 14, .textColor = { 180, 180, 190, 255 }
            })); }

            versors q = t->orientation;
            f32 pitch = asinf(2.0f * (q.y * q.z - q.x * q.w));
            f32 yaw   = atan2f(2.0f * (q.x * q.z + q.y * q.w),
                               q.x * q.x + q.y * q.y - q.z * q.z - q.w * q.w);
            f32 roll  = atan2f(2.0f * (q.x * q.y + q.z * q.w),
                               q.x * q.x - q.y * q.y - q.z * q.z + q.w * q.w);
            n = snprintf(buf, sizeof(buf), "Rotation:  %.0f, %.0f, %.0f",
                         pitch * (180.0f / PI), yaw * (180.0f / PI), roll * (180.0f / PI));
            { Clay_String s = { .length = n, .chars = buf };
            CLAY_TEXT(s, CLAY_TEXT_CONFIG({
                .fontId = 0, .fontSize = 14, .textColor = { 180, 180, 190, 255 }
            })); }
        }

        CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(1) } },
               .backgroundColor = { 80, 80, 90, 255 } }) {}

        n = snprintf(buf, sizeof(buf), "Components");
        { Clay_String s = { .length = n, .chars = buf };
        CLAY_TEXT(s, CLAY_TEXT_CONFIG({
            .fontId = 0, .fontSize = 14, .textColor = { 140, 200, 255, 255 }
        })); }

        static const char *labels[] = {
            [ECS_CMP_TRANSFORM_IDX] = "Transform",
            [ECS_CMP_MODEL_IDX]     = "Model",
            [ECS_CMP_INPUT_IDX]     = "Input",
            [ECS_CMP_MATERIAL_IDX]  = "Material",
            [ECS_CMP_CAMERA_IDX]    = "Camera",
            [ECS_CMP_COLLIDER_IDX]  = "Collider",
            [ECS_CMP_MESH_IDX]      = "Mesh",
        };
        for (u32 ci = 0; ci < ECS_CMP_COUNT; ci++) {
            if (entity_sig & (1 << ci)) {
                const char *label = labels[ci] ? labels[ci] : "?";
                n = snprintf(buf, sizeof(buf), "  %s", label);
                { Clay_String s = { .length = n, .chars = buf };
                CLAY_TEXT(s, CLAY_TEXT_CONFIG({
                    .fontId = 0, .fontSize = 13, .textColor = { 160, 220, 160, 255 }
                })); }
            }
        }
    }
}
