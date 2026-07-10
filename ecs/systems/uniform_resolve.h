#pragma once
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/types.h"
#include "poglib/ecs/uniform_registry.h"

void ecs_system_uniform_resolve(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_MATERIAL_IDX);

    slot_iterator(primary_pool, ITER)
    {
        const ecs_component_poolentry_t *const entry = ITER;
        ecs_component_material_t *material = (ecs_component_material_t *)entry->entity_cmpdata;

        if (!material->shader.asset_id) continue;

        uniform_registry_resolve_material_uniforms(
            material, cmp_manager, entry->entity_id, ctx.active_camera);
    }
}
