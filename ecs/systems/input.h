#pragma once
#include "../common.h"
#include "../component/types.h"
#include "poglib/poggen.h"
#include "poglib/poggen/input/commandqueue.h"

void ecs_system_input(slot_t * const componentpool, const ecs_componentmanager_t * const cmp_manager)
{
    ASSERT(global_poggen);
    (void)cmp_manager;

    const commandqueue_t *const commandqueue = &global_poggen->current_scene->commandqueue;

    slot_iterator(componentpool, iter)
    {
       ecs_component_input_t *const input_cmp = (ecs_component_input_t *)iter;
       const u16 bitmask = (u16)commandqueue_get_commands_as_bitmask(commandqueue);
       input_cmp->callback(input_cmp, bitmask);
    }
}
