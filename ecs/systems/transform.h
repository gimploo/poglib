#pragma once
#include "../component/types.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"

void ecs_system_transfrom__internal_source_manual(
        ecs_component_transform_t *const transform,
        ecs_component_input_t *const input
) {
    if (input == NULL) eprint("missing input component");

    const vec3f_t movement = input->internal.state.movement;
    const vec3s r = transform->orientation;
    const vec3s forward = {
        .x = cosf(r.y) * cosf(r.x),
        .y = sinf(r.x),
        .z = sinf(r.y) * cosf(r.x),
    };
    const vec3s right = {
        .x = cosf(r.y + M_PI_2) * cosf(r.x),
        .z = sinf(r.y + M_PI_2) * cosf(r.x),
    };
    const vec3s up = glms_vec3_cross(right, forward);

    vec3s delta = {0};
    delta = glms_vec3_add(delta, glms_vec3_scale(forward, movement.z));
    delta = glms_vec3_add(delta, glms_vec3_scale(right,   movement.x));
    delta = glms_vec3_add(delta, glms_vec3_scale(up,      movement.y));

    transform->position     = glms_vec3_add(delta, transform->position);
    transform->orientation  = glms_vec3_add(input->internal.state.rotation, transform->orientation);
}

void ecs_system_transform(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_TRANSFORM_IDX);

    slot_iterator(pool, iter)
    {
        ecs_component_poolentry_t *const entry = iter;
        ecs_component_transform_t *transform = (ecs_component_transform_t *)entry->entity_cmpdata;
        const ecs_entity_view_t view = ecs_componentmanager__internal_query_components(
            cmp_manager, 
            entry->entity_id, 
            ECS_CMP_INPUT);

        switch(transform->source)
        {
            case ECS_CMP_TRANSFORM_SOURCE_MANUAL:
                ecs_system_transfrom__internal_source_manual(
                    transform, 
                    view.entity_cmp_data[ECS_CMP_INPUT_IDX]);
            break;
            default: eprint("transform source not accounted for");
        }
    }
}
