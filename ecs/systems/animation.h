#pragma once
#include "poglib/ecs/common.h"
#include "poglib/poggen.h"

//TODO: do we need this really ??,
//Problem arose when its required for cloth sim to happen after animation but cloth sim needs
//to be set for the phys update. This created a problem to use for the character model, since
//we wont be using that anyways is this really needed for anything else - cant we just get 
//rid of it.
void ecs_system_animation(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_ANIMATION_IDX);
    slot_iterator(primary_pool, ITER)
    {
        const ecs_component_poolentry_t * const entry = (void *)ITER;
        ecs_component_animation_t *cmp_animation = (ecs_component_animation_t *)entry->entity_cmpdata;
#if 0

        glmodel_t *model = cmp_animation->internal.model;
        if (!model) {
            model = (glmodel_t *)assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_MODEL, cmp_animation->asset_id);
            cmp_animation->internal.model = model;
        }

        if (!model) continue;

        glmodel_update(model, APPLICATION_UPDATE_FIXED_TIME_STEP);
#endif
    }
}
