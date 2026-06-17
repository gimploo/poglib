#pragma once
#include "../component/types.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"

void ecs_system_transfrom__internal_source_manual(
        ecs_component_transform_t *const transform,
        ecs_component_input_t *const input
) {
    if (input == NULL) eprint("missing input component");

    transform->position     = input->internal.state.current_position;
    transform->orientation  = glms_quat_normalize(input->internal.state.current_orientation);
}

void ecs_system_transform(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    (void)ctx;

    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_TRANSFORM_IDX);
    slot_iterator(pool, iter)
    {
        ecs_component_poolentry_t *const entry = iter;
        ecs_component_transform_t *transform = (ecs_component_transform_t *)entry->entity_cmpdata;

        if (!entry->is_active) continue;

        switch(transform->source)
        {
            case ECS_CMP_TRANSFORM_SOURCE_INPUT:
                ecs_system_transfrom__internal_source_manual(
                    transform, 
                    ecs_componentmanager__internal_query_components(
                        cmp_manager, entry->entity_id, ECS_CMP_INPUT
                    ).entity_cmp_data[ECS_CMP_INPUT_IDX]
                );
            break;

            case ECS_CMP_TRANSFORM_SOURCE_ANIMATION:    //NOTE: animation system updates the position and orientation directly 
            case ECS_CMP_TRANSFORM_SOURCE_PHYSICS:      //NOTE: physics system updates the position and orientation directly 
            case ECS_CMP_TRANSFORM_SOURCE_NONE:
            break;

            default: eprint("transform source not accounted for");
        }
    }
}
