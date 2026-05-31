#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>

#include "./common.h"
#include "./component/types.h"
#include "poglib/basic/common.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/basic/ds/slot.h"
#include "poglib/basic/util.h"


ecs_componentmanager_t  ecs_componentmanager(arena_t *arena);

void                    ecs_componentmanager_add(ecs_componentmanager_t * const self, const u32 entityId, const ecs_componentbundle_t component_config);
void                    ecs_componentmanager_remove(ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type type);
void                    ecs_componentmanager_removeall(ecs_componentmanager_t * const self, const u32 entity_id);

ecs_component_entry_t   ecs_componentmanager_get_component(const ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type cmp_type);
ecs_entity_view_t       ecs_componentmanager_query_components(const ecs_componentmanager_t * const self, const u32 entity_id, const u32 component_signature);


#ifndef IGNORE_ECS_COMPONENT_IMPLEMENTATION

u16 ecs_component__internal_get_componenttype_size(ecs_component_type type)
{
    switch(type)
    {
        case ECS_CMP_TRANSFORM:
            return sizeof(ecs_component_transform_t);
        break;
        case ECS_CMP_MODEL:
            return sizeof(ecs_component_model_t);
        break;
        case ECS_CMP_INPUT:
            return sizeof(ecs_component_input_t);
        break;
        case ECS_CMP_MATERIAL:
            return sizeof(ecs_component_material_t);
        break;
        case ECS_CMP_CAMERA:
            return sizeof(ecs_component_camera_t);
        default: eprint("missing component type - not implemented");
    }
}

ecs_componentmanager_t ecs_componentmanager(arena_t *arena)
{
    ecs_componentmanager_t o = {
        .componentpool_slots = slot_init(ECS_CMP_COUNT, sizeof(slot_t), arena),
        .entity2components_lookup = hashtable_init(
            ECS_ENTITY_MAX_COUNT,
            HT_KEY_TYPE_U32,
            (ht_value_type) {
                .type = HT_STORAGE_BY_VALUE,
                .size = sizeof(i16[ECS_CMP_COUNT])
            },
            arena
        ),
    };

    slot_fill_empty(&o.componentpool_slots);

    for (u16 comp_idx = 0; comp_idx < ECS_CMP_COUNT; comp_idx++)
    {
        const ecs_component_type componenttype = 1 << comp_idx;
        slot_t * const packedcmp_slot          = slot_get_value(&o.componentpool_slots, comp_idx);

        //NOTE: pool data - since cmp_size is dynamic, we cant have struct to encapulate this easily
        // u32      / <cmp_size>
        // [ entity_id ][ component data ]
        *packedcmp_slot = slot_init(
            ECS_ENTITY_MAX_COUNT, 
            sizeof(u32) + ecs_component__internal_get_componenttype_size(componenttype), 
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


void ecs_componentmanager_add(ecs_componentmanager_t * const self, const u32 entity_id, const ecs_componentbundle_t config)
{
    const u32 cmp_count = ecs_componentmanager__internal_get_component_count(config.signature);
    ASSERT(cmp_count > 0);

    i16 *cmp_indexes = hashtable_has_key(&self->entity2components_lookup, (hashtable_key_t){ .u32 = entity_id }) 
        ? (i16 *)hashtable_get_value(&self->entity2components_lookup, (hashtable_key_t){ .u32 = entity_id })
        : NULL;

    i16 cmp_idx_buffer[ECS_CMP_COUNT];
    if (cmp_indexes)    memcpy(cmp_idx_buffer, cmp_indexes, sizeof(cmp_idx_buffer));
    else                ARRAY_FILL(cmp_idx_buffer, ECS_CMP_INVALID_IDX);

    i16 cmp_idx_count = ECS_CMP_INVALID_IDX;
    while(true)
    {
        if (++cmp_idx_count == ECS_CMP_COUNT)
            break;

        const ecs_component_type cmp_type               = 1 << cmp_idx_count;
        const bool component_type_exist_in_signature    = (config.signature & cmp_type);
        if (!component_type_exist_in_signature)
            continue;

        slot_t *const pool                  = slot_get_value(&self->componentpool_slots, cmp_idx_count);
        cmp_idx_buffer[cmp_idx_count]       = pool->len;
        const u16 cmp_size                  = ecs_component__internal_get_componenttype_size(cmp_type);

        buffer(WORD) buf = {0};
        memcpy(buf.raw_data, &entity_id, sizeof(entity_id));
        memcpy((u8 *)buf.raw_data + sizeof(entity_id) , &config.component[cmp_idx_count], cmp_size);
        slot_insert(pool, pool->len, buf.raw_data, sizeof(u32) + cmp_size);
    }

    hashtable_insert(
        &self->entity2components_lookup, 
        (hashtable_key_t){ .u32 = entity_id }, 
        cmp_idx_buffer
    );
}

u32 ecs_componentmanager__internal_get_entity_id_from_pooldata(const void * const data)
{
    return *((u32 *)data);
}

void * ecs_componentmanager__internal_get_cmpdata_from_pooldata(const void * const data)
{
    void * const pooldata = (u8 *)data + sizeof(u32);
    return pooldata;
}

void ecs_componentmanager_remove(ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type type)
{
    const u32 cmp_idx           = get_index_from_bitflag(type);
    slot_t *const componentpool = slot_get_value(&self->componentpool_slots, cmp_idx);

    if (!hashtable_has_key(&self->entity2components_lookup, (hashtable_key_t){ .u32 = entity_id })) {
        return;
    }

    i16 *cmp_idx_buffer = (i16 *)hashtable_get_value(
        &self->entity2components_lookup, 
        (hashtable_key_t){ .u32 = entity_id }
    );

    const u16 index_of_cmp_to_remove                = cmp_idx_buffer[cmp_idx];
    const bool is_last_element                      = index_of_cmp_to_remove == (componentpool->len - 1);
    cmp_idx_buffer[cmp_idx]                         = ECS_CMP_INVALID_IDX;

    slot_delete(componentpool, index_of_cmp_to_remove);

    if (is_last_element) {
        return;
    }

    //NOTE: swaps last item to deleted item's index

    const u32 last_element_data_idx         = componentpool->len - 1;
    void *last_element_data                 = slot_get_value(componentpool, last_element_data_idx);
    const u32 moved_entity_id               = ecs_componentmanager__internal_get_entity_id_from_pooldata(last_element_data);
    i16 *const cmp_buf                      = hashtable_get_value(&self->entity2components_lookup, (hashtable_key_t){ .u32 = moved_entity_id });
    cmp_buf[get_index_from_bitflag(type)]   = index_of_cmp_to_remove;
    hashtable_insert(&self->entity2components_lookup, (hashtable_key_t){ .u32 = moved_entity_id }, cmp_buf);

    const u32 full_allocation_size          = sizeof(u32) + ecs_component__internal_get_componenttype_size(type);
    slot_insert(componentpool, index_of_cmp_to_remove, last_element_data, full_allocation_size);
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

ecs_component_entry_t ecs_componentmanager_get_component_from_pool(const slot_t *pool, const ecs_component_type pool_type, const u32 entity_id, const hashtable_t * const entity2components_lookup)
{
    const i16 *cmp_index_buffer = hashtable_get_value(entity2components_lookup, (hashtable_key_t){ .u32 = entity_id });
    const i16 cmp_index = cmp_index_buffer[get_index_from_bitflag(pool_type)];
    ASSERT(cmp_index >= 0);

    return (ecs_component_entry_t) {
        .type = pool_type,
        .entity_id = ecs_componentmanager__internal_get_entity_id_from_pooldata(slot_get_value(pool, cmp_index)),
        .data = ecs_componentmanager__internal_get_cmpdata_from_pooldata(slot_get_value(pool, cmp_index))
    };
}

ecs_component_entry_t ecs_componentmanager_get_component(const ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type cmp_type)
{
    ASSERT(self);
    const u32 cmptype_index = get_index_from_bitflag(cmp_type);

    const slot_t *cmp_pool = slot_get_value(&self->componentpool_slots, cmptype_index);
    ASSERT(cmp_pool);

    return ecs_componentmanager_get_component_from_pool(cmp_pool, cmp_type, entity_id, &self->entity2components_lookup);

}

ecs_entity_view_t ecs_componentmanager_query_components(const ecs_componentmanager_t * const self, const u32 entity_id, const u32 component_signature)
{
    ASSERT(self);
    ASSERT(entity_id >= 0);
    ASSERT(component_signature > 0);

    ecs_entity_view_t result = {0};

    i16 *cmp_idx_buffer = hashtable_get_value(&self->entity2components_lookup, (hashtable_key_t){ .u32 = entity_id });

    for (u16 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        if (cmp_idx_buffer[cmp_idx] != ECS_CMP_INVALID_IDX && (component_signature & (1 << cmp_idx)) == 0)
            continue;

        const slot_t *const pool = slot_get_value(&self->componentpool_slots, cmp_idx);
        result.entity_cmp_data[cmp_idx] = ecs_componentmanager__internal_get_cmpdata_from_pooldata(
            slot_get_value(pool, cmp_idx_buffer[cmp_idx])
        );
    }

    return result;
}


#endif

