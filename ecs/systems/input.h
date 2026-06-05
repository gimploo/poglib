#pragma once
#include "../common.h"
#include "../component/types.h"
#include "poglib/application.h"
#include "poglib/basic/ds/slot.h"
#include "poglib/poggen.h"

void ecs_system_input(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    ASSERT(global_poggen);
    ASSERT(ctx.active_commandqueue);

    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_INPUT_IDX);
    slot_iterator(pool, iter)
    {
        const ecs_component_poolentry_t * const entry   = iter;
        ecs_component_input_t *const input_cmp          = (ecs_component_input_t *)entry->entity_cmpdata;

        input_cmp->internal.state = (ecs_component_input_state_t){0};

        if (!entry->is_active) continue;

        input_cmp->input_behavior(
            &input_cmp->internal.state, 
            commandqueue_get_commands_as_bitmask(ctx.active_commandqueue), 
            APPLICATION_UPDATE_FIXED_TIME_STEP
        );
    }
}
