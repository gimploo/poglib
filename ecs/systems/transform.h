#pragma once
#include "../component/types.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/external/cglm/struct/io.h"

void ecs_system_transfrom__internal_source_manual(
        ecs_component_transform_t *const transform,
        ecs_component_input_t *const input
) {
    transform->translation.x += input->state.move_dir.x;
    transform->translation.y += input->state.move_dir.y;
    transform->translation.z += input->state.move_dir.z;
}

void ecs_system_transform(ecs_componentmanager_t *const cmp_manager)
{
    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_TRANSFORM_IDX);
    slot_iterator(pool, iter)
    {
        ecs_component_poolentry_t *const entry = iter;
        ecs_component_transform_t *transform = (ecs_component_transform_t *)entry->entity_cmpdata;
        const ecs_entity_view_t view = ecs_componentmanager_query_components(
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
