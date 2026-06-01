#pragma once
#include "../common.h"
#include "../component/types.h"
#include "poglib/application.h"
#include "poglib/basic/ds/slot.h"
#include "poglib/poggen.h"
#include "poglib/poggen/input/commandqueue.h"

void ecs_system_input(ecs_componentmanager_t *const cmp_manager)
{
    ASSERT(global_poggen);
    (void)cmp_manager;

    slot_t * const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_INPUT_IDX);
    slot_iterator(pool, iter)
    {
        const ecs_component_poolentry_t * const entry   = iter;
        ecs_component_input_t *const input_cmp          = (ecs_component_input_t *)entry->entity_cmpdata;
        const u16 bitmask                               = (u16)commandqueue_get_commands_as_bitmask(&global_poggen->current_scene->commandqueue);

        input_cmp->state = (ecs_component_input_state_t){0};
        input_cmp->input_behavior(&input_cmp->state, bitmask, APPLICATION_UPDATE_FIXED_TIME_STEP);
    }
}
