#pragma once
#include "poglib/ecs/common.h"
#include "poglib/ecs/component/types.h"
#include "poglib/ecs/component/material_uniform_resolver.h"
#include "poglib/ecs/component/material_texture_resolver.h"

//TODO: MATERIAL UNIFORM SHADER AUTO DETECT - have shader uniforms be auto detected during init, have the assetmanager load the file and pass the uniform setup
//in within the ecs, such that all ecs uniform lookup types can be checked without the dev having to set it during 
//load time, and only add on top of it with additional uniforms passed in manually


void ecs_system_material(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *const primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_MATERIAL_IDX);

    slot_iterator(primary_pool, ITER)
    {
        const ecs_component_poolentry_t *const entry    = (ecs_component_poolentry_t *)ITER;
        ecs_component_material_t *const material        = (ecs_component_material_t *)entry->entity_cmpdata;

        if (!material->shader_asset_id) continue;

        //TODO: if uniforms are already resolved once used the cached data to only 
        //update values instead of checking the shader to know if the uniform exist in
        //it also

        ecs_system_material__internal__resolve_lighting(
            material,
            slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_DIRECTIONAL_LIGHT_IDX)
        );

        ecs_system_material__internal__resolve_textures_from_model(
            material,
            cmp_manager,
            entry->entity_id
        );

        ecs_system_material__internal__resolve_uniforms(
            material, cmp_manager, entry->entity_id, ctx.active_camera
        );

    }
}
