#pragma once
#include "poglib/basic/ds/hashtable.h"
#include "poglib/basic/ds/slot.h"
#include <poglib/basic.h>
#include "./common.h"

typedef struct ecs_entity_t ecs_entity_t;
struct ecs_entity_t {
    u32 id;
    u32 component_signature;
};

ecs_entitymanager_t ecs_entitymanager(arena_t * const arena)
{
    ASSERT(arena);
    return (ecs_entitymanager_t) {
        .entities = slot_init(ECS_ENTITY_MAX_COUNT, sizeof(ecs_entity_t), arena),
        .lookup = hashtable_init(ECS_ENTITY_MAX_COUNT, HT_KEY_TYPE_U32, (ht_value_type){ .size = sizeof(u32), .type = HT_STORAGE_BY_VALUE_INLINE }, arena)
    };
}

void ecs_entitymanager_add(ecs_entitymanager_t * const self, const ecs_entity_t entityconfig)
{
    slot_append(
        &self->entities, 
        entityconfig
    );

    hashtable_insert(
        &self->lookup, 
        (hashtable_key_t){ .u32 = entityconfig.id }, 
        self->entities.len - 1
    );
}

void ecs_entitymanager_remove(ecs_entitymanager_t * const self, const u32 entityId)
{
    const u32 remove_entity_idx = (u32)hashtable_get_value(&self->lookup, (hashtable_key_t){ .u32 = entityId });
    const bool is_entity_at_far_end = remove_entity_idx == (self->entities.len - 1);

    hashtable_delete(&self->lookup, (hashtable_key_t){ .u32 = entityId });
    slot_delete(&self->entities, remove_entity_idx);

    if (is_entity_at_far_end) {
        return;
    }

    //NOTE: swap last entity in the list to removed index
    const u32 swap_entity_idx = self->entities.len - 1;
    const ecs_entity_t *swap_entity =  slot_get_value(&self->entities, swap_entity_idx);

    slot_insert(&self->entities, remove_entity_idx, swap_entity, sizeof(ecs_entity_t));
    slot_delete(&self->entities, swap_entity_idx);

    hashtable_insert(&self->lookup, (hashtable_key_t){ .u32 = swap_entity->id }, swap_entity_idx);
}

