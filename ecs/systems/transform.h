#pragma once
#include "../component/types.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"

void ecs_system_transfrom__internal_source_manual(
        ecs_component_transform_t * const transform,
        const ecs_component_input_t input
) {
    transform->translation.x += input.move_dir.x * input.speed.x * input.delta_time;
    transform->translation.y += input.move_dir.y * input.speed.y * input.delta_time;
    transform->translation.z += input.move_dir.z * input.speed.z * input.delta_time;
}

void ecs_system_transform(const ecs_componentmanager_t * const cmp_manager)
{
    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_TRANSFORM_IDX);
    slot_iterator(pool, component)
    {
    }
}
