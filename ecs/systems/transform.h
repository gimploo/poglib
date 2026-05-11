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

void ecs_system_transform(slot_t * const componentpool, const ecs_componentmanager_t * const cmp_manager)
{
    slot_iterator(componentpool, component)
    {
        ecs_component_transform_t * const transform = component;

        switch(transform->source)
        {
            case ECS_CMP_TRANSFORM_SOURCE_MANUAL:
                ecs_system_transfrom__internal_source_manual(
                    transform, 
                    *(ecs_component_input_t *)ecs_componentmanager_get_component(
                        cmp_manager, 
                        transform->internal.entity_id, 
                        ECS_CMP_INPUT)
                );
            break;
            case ECS_CMP_TRANSFORM_SOURCE_PHYSICS:
                eprint("not implemented");
            break;
            case ECS_CMP_TRANSFORM_SOURCE_ANIMATION:
                eprint("not implemented");
            break;
            default: eprint("unknown transform source type");
        }
    }
}
