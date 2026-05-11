#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>

#include "./common.h"
#include "./component/types.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/basic/ds/slot.h"

ecs_componentmanager_t  ecs_componentmanager(arena_t *arena);
void                    ecs_componentmanager_add(ecs_componentmanager_t * const self, const u32 entityId, const ecs_componentbundle_t component_config);
void                    ecs_componentmanager_remove(ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type type);
void                    ecs_componentmanager_removeall(ecs_componentmanager_t * const self, const u32 entity_id);
void *                  ecs_componentmanager_get_component(const ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type cmp_type);
ecs_query_entitycmps_t  ecs_componentmanager_query_components(const ecs_componentmanager_t * const self, const u32 entity_id, const u32 component_signature);


#ifndef IGNORE_ECS_COMPONENT_IMPLEMENTATION


typedef struct ecs__internal_cmp_lookupindex_t ecs__internal_cmp_lookupindex_t;
struct ecs__internal_cmp_lookupindex_t {
    u16 cmp_buffer_index[ECS_CMP_COUNT];
};

u16 ecs_component__internal_get_componenttype_size(ecs_component_type type)
{
    switch(type)
    {
        case ECS_CMP_TRANSFORM:
            return sizeof(ecs_component_transform_t);
        break;
        case ECS_CMP_MESH:
            return sizeof(ecs_component_mesh_t);
        break;
        case ECS_CMP_INPUT:
            return sizeof(ecs_component_input_t);
        break;
        default: eprint("missing component type - not implemented");
    }
}

ecs_componentmanager_t ecs_componentmanager(arena_t *arena)
{
    ecs_componentmanager_t o = {
        .componentpool_slots = slot_init(ECS_CMP_COUNT, sizeof(slot_t), arena),
        .entity2component_lookup_slots = slot_init(ECS_CMP_COUNT, sizeof(hashtable_t), arena),
    };

    slot_fill_empty(&o.componentpool_slots);
    slot_fill_empty(&o.entity2component_lookup_slots);

    for (u16 comp_idx = 0; comp_idx < ECS_CMP_COUNT; comp_idx++)
    {
        const ecs_component_type componenttype = 1 << comp_idx;
        slot_t * const packedcmp_slot          = slot_get_value(&o.componentpool_slots, comp_idx);
        hashtable_t * const cmp_entity_lookup  = slot_get_value(&o.entity2component_lookup_slots, comp_idx);

        *packedcmp_slot = slot_init(
            MAX_ENTITY_COUNT, 
            ecs_component__internal_get_componenttype_size(componenttype), 
            arena
        );

        *cmp_entity_lookup = hashtable_init(
            MAX_ENTITY_COUNT, 
            HT_KEY_TYPE_U32, 
            u32, 
            arena
        );
    }

    return o;
}

//Brian Kernighan’s Algorithm.
u32 ecs_componentmanager__internal_get_component_count(u32 signature) {
    u32 count = 0;
    while (signature > 0) {
        signature &= (signature - 1);
        count++;
    }
    return count;
}


void ecs_componentmanager_add(ecs_componentmanager_t * const self, const u32 entityId, const ecs_componentbundle_t config)
{
    const u32 cmp_count = ecs_componentmanager__internal_get_component_count(config.signature);
    ASSERT(cmp_count > 0);

    i32 cmp_idx_count = -1;
    while(true)
    {
        if (++cmp_idx_count == ECS_CMP_COUNT)
            break;

        const ecs_component_type cmp_type = 1 << cmp_idx_count;
        const bool component_type_exist_in_signature = (config.signature & cmp_type) != 0;

        if (!component_type_exist_in_signature)
            continue;


        slot_t *const pool          = slot_get_value(&self->componentpool_slots, cmp_idx_count);
        hashtable_t *const lookup   = slot_get_value(&self->entity2component_lookup_slots, cmp_idx_count);
        const u16 cmp_size          = ecs_component__internal_get_componenttype_size(cmp_type);

        const u32 new_index = pool->len;
        slot_insert(pool, pool->len, &config.component[cmp_idx_count], cmp_size);

        hashtable_insert(
            lookup, 
            (hashtable_key_t){ .u32 = entityId }, 
            new_index
        );
    }
}

void ecs_componentmanager_remove(ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type type)
{
    const u16 cmp_idx = get_index_from_bitflag(type);

    slot_t *const componentpool = slot_get_value(&self->componentpool_slots, cmp_idx);
    hashtable_t *const entity2cmplookup = slot_get_value(&self->entity2component_lookup_slots, cmp_idx);

    if (!hashtable_has_key(entity2cmplookup, (hashtable_key_t){ .u32 = entity_id })) {
        return;
    }

    const u16 index_of_cmp_to_remove = (u16)hashtable_get_value(
        entity2cmplookup, 
        (hashtable_key_t){ .u32 = entity_id }
    );

    hashtable_delete(entity2cmplookup, (hashtable_key_t){ .u32 = entity_id });

    const bool is_last_element = index_of_cmp_to_remove == (componentpool->len - 1);
    slot_delete(componentpool, index_of_cmp_to_remove);
    hashtable_delete(entity2cmplookup, (hashtable_key_t){ .u32 = entity_id });

    if (is_last_element) {
        return;
    }

    //NOTE: swaps last item to delted item's index

    const u32 last_element_data_idx     = componentpool->len - 1;
    void *last_element_data             = slot_get_value(componentpool, last_element_data_idx);
    const u32 moved_entity_id           = hashtable_get_key_from_value(entity2cmplookup, last_element_data_idx).u32;

    hashtable_insert(entity2cmplookup, (hashtable_key_t){ .u32 = moved_entity_id }, index_of_cmp_to_remove);
    slot_insert(componentpool, index_of_cmp_to_remove, last_element_data, ecs_component__internal_get_componenttype_size(type));
    slot_delete(componentpool, last_element_data_idx);
}


void ecs_componentmanager_removeall(ecs_componentmanager_t * const self, const u32 entity_id)
{
    for (u16 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        const ecs_component_type type = 1 << cmp_idx;
        ecs_componentmanager_remove(
            self, 
            entity_id, 
            type
        );
    }
}

void * ecs_componentmanager_get_component(const ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type cmp_type)
{
    ASSERT(self);

    const hashtable_t *table = (u16)slot_get_value(&self->entity2component_lookup_slots, 1 << cmp_type);
    ASSERT(table);

    const slot_t *cmp_pool = slot_get_value(&self->componentpool_slots, 1 << cmp_type);
    ASSERT(cmp_pool);

    const u16 cmp_index = (u16)hashtable_get_value(table, (hashtable_key_t){ .u32 = entity_id });
    return slot_get_value(cmp_pool, cmp_index);
}

ecs_query_entitycmps_t ecs_componentmanager_query_components(const ecs_componentmanager_t * const self, const u32 entity_id, const u32 component_signature)
{
    ASSERT(self);
    ASSERT(entity_id >= 0);
    ASSERT(component_signature > 0);

    ecs_query_entitycmps_t result = {0};

    for (u16 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        const bool has_component = !(component_signature & (1 << cmp_idx));
        if (!has_component) {
            continue;
        }
        result.hit_count++;
        result.cmps[cmp_idx] = ecs_componentmanager_get_component(self, entity_id, 1 << cmp_idx);
    }

    return result;
}


#endif

