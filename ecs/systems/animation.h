#pragma once
#include "poglib/ecs/common.h"
#include "poglib/gfx/model/assimp.h"
#include "poglib/poggen.h"

void ecs_system_animation(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_MODEL_IDX);

    slot_iterator(primary_pool, ITER)
    {
        const ecs_component_poolentry_t * const entry = ITER;
        ecs_component_model_t *cmp_model = (ecs_component_model_t *)entry->entity_cmpdata;

        if (!cmp_model->internal.model) {
            cmp_model->internal.model = (glmodel_t *)assetmanager_get_assetresource(
                &global_engine->systems.assets, 
                ASSET_TYPE_MODEL,
                (cmp_model)->asset_id
            );
        }

        if (!cmp_model->internal.model) {
            continue;
        }

        glmodel_play_animation(cmp_model->internal.model, APPLICATION_UPDATE_FIXED_TIME_STEP);
    }

}
