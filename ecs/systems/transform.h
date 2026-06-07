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
    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_TRANSFORM_IDX);

    slot_iterator(pool, iter)
    {
        ecs_component_poolentry_t *const entry = iter;
        ecs_component_transform_t *transform = (ecs_component_transform_t *)entry->entity_cmpdata;

        if (!entry->is_active) continue;

        switch(transform->source)
        {
            case ECS_CMP_TRANSFORM_SOURCE_MANUAL:
            {
                const ecs_entity_view_t view = ecs_componentmanager__internal_query_components(
                    cmp_manager, entry->entity_id, ECS_CMP_INPUT);
                ecs_system_transfrom__internal_source_manual(
                    transform, 
                    view.entity_cmp_data[ECS_CMP_INPUT_IDX]);
            } break;
            case ECS_CMP_TRANSFORM_SOURCE_PHYSICS: {
                const ecs_entity_view_t cv = ecs_componentmanager__internal_query_components(
                    cmp_manager, entry->entity_id, ECS_CMP_CHARACTER);
                ecs_component_character_t *collision = cv.entity_cmp_data[ECS_CMP_CHARACTER_IDX];
                if (collision && collision->character) {
                    JPH_RVec3 phys_pos;
                    JPH_Quat phys_rot;
                    JPH_CharacterVirtual_GetPosition(collision->character, &phys_pos);
                    JPH_CharacterVirtual_GetRotation(collision->character, &phys_rot);
                    const vec3f_t pos = {phys_pos.x, phys_pos.y, phys_pos.z};
                    transform->position = pos;
                    transform->orientation = (versors){phys_rot.x, phys_rot.y, phys_rot.z, phys_rot.w};
                }
            } break;
            case ECS_CMP_TRANSFORM_SOURCE_NONE:
            case ECS_CMP_TRANSFORM_SOURCE_ANIMATION:
            break;
            default: eprint("transform source not accounted for");
        }
    }
}
