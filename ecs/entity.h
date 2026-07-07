#pragma once
#include "poglib/basic/ds/hashtable.h"
#include "poglib/basic/ds/slot.h"
#include <poglib/basic.h>
#include "./common.h"

ecs_entitymanager_t ecs_entitymanager(arena_t * const arena)
{
    ASSERT(arena);
    return (ecs_entitymanager_t) {
        .entities = slot_init(ECS_ENTITY_MAX_COUNT, sizeof(ecs_entity_t), arena),
        .entityid_to_entityidx_lookup = hashtable_init(ECS_ENTITY_MAX_COUNT, HT_KEY_TYPE_U32, (ht_value_type){ .size = sizeof(u32), .type = HT_STORAGE_BY_VALUE_INLINE }, arena)
    };
}

bool ecs_entitymanager_does_entity_exist(const ecs_entitymanager_t *const self, const u32 entity_id)
{
    return hashtable_has_key(
        &self->entityid_to_entityidx_lookup, 
        (hashtable_key_t){ .u32 = entity_id} 
    );
}

ecs_entity_t ecs_entitymanager_get_entity(const ecs_entitymanager_t *const self, const u32 entity_id)
{
    const u64 entity_idx = (u64)hashtable_get_value(
        &self->entityid_to_entityidx_lookup, 
        (hashtable_key_t){ .u32 = entity_id} 
    );

    return *(ecs_entity_t *)slot_get_value(&self->entities, entity_idx);
}

void ecs_entitymanager_add(ecs_entitymanager_t * const self, const ecs_entity_t entityconfig)
{
    slot_append(
        &self->entities, 
        entityconfig
    );

    hashtable_insert(
        &self->entityid_to_entityidx_lookup, 
        (hashtable_key_t){ .u32 = entityconfig.id }, 
        self->entities.len - 1
    );
}

void ecs_entitymanager_remove(ecs_entitymanager_t * const self, const u32 entityId)
{
    const u32 remove_entity_idx = (u32)hashtable_get_value(&self->entityid_to_entityidx_lookup, (hashtable_key_t){ .u32 = entityId });
    const bool is_entity_at_far_end = remove_entity_idx == (self->entities.len - 1);

    hashtable_delete(&self->entityid_to_entityidx_lookup, (hashtable_key_t){ .u32 = entityId });

    if (is_entity_at_far_end) {
        slot_delete(&self->entities, remove_entity_idx);
        return;
    }

    //NOTE: swap last entity in the list to removed index
    const u32 swap_entity_idx = self->entities.len - 1;
    const ecs_entity_t *swap_entity =  slot_get_value(&self->entities, swap_entity_idx);

    slot_delete(&self->entities, remove_entity_idx);
    slot_insert(&self->entities, remove_entity_idx, swap_entity, sizeof(ecs_entity_t));
    slot_delete(&self->entities, swap_entity_idx);
    hashtable_insert(&self->entityid_to_entityidx_lookup, (hashtable_key_t){ .u32 = swap_entity->id }, remove_entity_idx);
}

