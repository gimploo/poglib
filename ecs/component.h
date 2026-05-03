#pragma once
#include "poglib/basic/ds/hashtable.h"
#include "poglib/basic/ds/slot.h"
#include "poglib/basic/util.h"
#include "poglib/math/la.h"
#include <poglib/basic.h>
#include "./common.h"

//NOTE: update the count after updating the ecs component type
#define ECS_CMP_COUNT 2

typedef struct ecs_componentmanager_t {
    //NOTE: this is going to hold all data of component type 
    //packed together in a single array for a type, each type will 
    //have its own slot index
    slot_t componentpool_slots; 

    //NOTE: list of entityId to component index in packedcomponent_buffers 
    //individual lookup tables
    slot_t entity2component_lookup_slots;
} ecs_componentmanager_t;

typedef enum {
    ECS_CMP_TRANSFORM   = 1 << 0,
    ECS_CMP_MESH        = 1 << 1,
} ecs_component_type;

typedef struct ecs_component_transform_t {
    vec3f_t translation;
    vec3f_t rotation;
    vec3f_t scale;
} ecs_component_transform_t;

typedef struct ecs_component_mesh_t {
    //TODO: what would a mesh need, asset id probably?
} ecs_component_mesh_t;

ecs_componentmanager_t  ecs_componentmanager(arena_t *arena);
void                    ecs_componentmanager_add(ecs_componentmanager_t * const self, const u32 entityId, const u32 signature);
void                    ecs_componentmanager_remove(ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type type);
void                    ecs_componentmanager_removeall(ecs_componentmanager_t * const self, const u32 entity_id);


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
        default: eprint("missing component type - not implemented");
    }
}

ecs_componentmanager_t ecs_componentmanager(arena_t *arena)
{
    ecs_componentmanager_t o = {
        .componentpool_slots = slot_init(ECS_CMP_COUNT, sizeof(slot_t), false, arena),
        .entity2component_lookup_slots = slot_init(ECS_CMP_COUNT, sizeof(hashtable_t), false, arena),
    };

    for (u16 comp_idx = 0; comp_idx < ECS_CMP_COUNT; comp_idx++)
    {
        const ecs_component_type componenttype = 1 << comp_idx;

        slot_t *packedcmp_slot          = slot_get_value(&o.componentpool_slots, comp_idx);
        hashtable_t *cmp_entity_lookup  = slot_get_value(&o.entity2component_lookup_slots, comp_idx);

        *packedcmp_slot = slot_init(
            MAX_ENTITY_COUNT, 
            ecs_component__internal_get_componenttype_size(componenttype), 
            false, 
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

void ecs_componentmanager_add(ecs_componentmanager_t * const self, const u32 entityId, const u32 signature)
{
    for(u16 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        const ecs_component_type cmp_type = 1 << cmp_idx;

        const bool cmp_configured = (signature & (cmp_type)) == 0;
        if (!cmp_configured)
            continue;

        slot_t *const packedcmp_slot = slot_get_value(&self->componentpool_slots, cmp_idx);
        const u16 cmp_size = ecs_component__internal_get_componenttype_size(cmp_type);

        const u32 new_index = MAX(packedcmp_slot->len - 1, 0);
        slot_insert(packedcmp_slot, new_index, NULL, cmp_size);

        hashtable_t *const entity2cmplookup = slot_get_value(&self->componentpool_slots, cmp_idx);
        hashtable_insert(
            entity2cmplookup, 
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

    const u32 index_of_cmp_to_remove = (u32)hashtable_get_value(
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


#endif

