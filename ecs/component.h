#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>

#include "./common.h"
#include "./component/types.h"
#include "poglib/basic/common.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/basic/ds/slot.h"
#include "poglib/basic/util.h"
#include "poglib/ecs/component/colliderbatchqueue.h"


ecs_componentmanager_t          ecs_componentmanager(arena_t *arena);
void                            ecs_componentmanager_update(ecs_componentmanager_t * const self);
void                            ecs_componentmanager_add(ecs_componentmanager_t * const self, const u32 entityId, const ecs_componentbundle_t component_config);
void                            ecs_componentmanager_remove(ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type type);
void                            ecs_componentmanager_removeall(ecs_componentmanager_t * const self, const u32 entity_id);
ecs_component_entry_t           ecs_componentmanager_get_component(const ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type cmp_type);
void                            ecs_componentmanager_patch_entity_components(ecs_componentmanager_t *const self, const u32 entity_id, const ecs_cmp_patch_payload_t request);


#ifndef IGNORE_ECS_COMPONENT_IMPLEMENTATION

void ecs_componentmanager__internal_cmp_cleanup(const ecs_component_type type, const ecs_component_poolentry_t *poolentry);

u16 ecs_component__internal_get_componenttype_size(const ecs_component_type type)
{
    switch(type)
    {
        case ECS_CMP_TRANSFORM:         return sizeof(ecs_component_transform_t);
        case ECS_CMP_MODEL:             return sizeof(ecs_component_model_t);
        case ECS_CMP_INPUT:             return sizeof(ecs_component_input_t);
        case ECS_CMP_MATERIAL:          return sizeof(ecs_component_material_t);
        case ECS_CMP_CAMERA:            return sizeof(ecs_component_camera_t);
        case ECS_CMP_COLLIDER:          return sizeof(ecs_component_collider_t);
        default: eprint("missing component type - not implemented");
    }
}

u32 ecs_componentmanager__internal_get_pool_capacity(const ecs_component_type type)
{
    switch(type)
    {
        case ECS_CMP_CAMERA:            return 10;
        default:                        return ECS_ENTITY_MAX_COUNT / 2;
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
        .internal = {
            .colliderbatch = colliderbatchqueue(arena)
        }
    };

    slot_fill_empty(&o.componentpool_slots);

    for (u16 comp_idx = 0; comp_idx < ECS_CMP_COUNT; comp_idx++)
    {
        const ecs_component_type componenttype = 1 << comp_idx;
        slot_t * const packedcmp_slot          = slot_get_value(&o.componentpool_slots, comp_idx);

        //NOTE: pool data - since cmp_size is dynamic, we cant have struct to encapulate this easily
        // u32          / bool      / <cmp_size>
        // [ entity_id ][is_active][ component data ]
        *packedcmp_slot = slot_init(
            ecs_componentmanager__internal_get_pool_capacity(componenttype),
            ECS_CMP_POOL_HEADER_SIZE + ecs_component__internal_get_componenttype_size(componenttype), 
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


void ecs_component__internal_bundle_validate_and_initalize_internals(ecs_componentbundle_t *const config)
{
    i32 idx = ECS_CMP_INVALID_IDX;
    while(true)
    {
        if (++idx == ECS_CMP_COUNT)
            break;

        const ecs_component_type cmp_type = 1 << idx;
        if (!(config->signature & cmp_type))
            continue;

        switch(cmp_type)
        {
            case ECS_CMP_TRANSFORM: {
                if (config->component[ECS_CMP_TRANSFORM_IDX].transform.orientation.w == 0) {
                    config->component[ECS_CMP_TRANSFORM_IDX].transform.orientation = GLM_QUAT_IDENTITY;
                }
                if (config->component[ECS_CMP_TRANSFORM_IDX].transform.source == ECS_CMP_TRANSFORM_SOURCE_PHYSICS) {
                    ASSERT(config->signature & ECS_CMP_COLLIDER);
                }
            } break;
            case ECS_CMP_COLLIDER: {
                ASSERT(config->signature & (ECS_CMP_TRANSFORM));
                const ecs_component_transform_t t = config->component[ECS_CMP_TRANSFORM_IDX].transform;
                config->component[ECS_CMP_COLLIDER_IDX].collider.internal.orientation = t.orientation;
                config->component[ECS_CMP_COLLIDER_IDX].collider.internal.position = t.position;
            } break;

            case ECS_CMP_MODEL: 
                ASSERT(config->signature & (ECS_CMP_TRANSFORM));
                ASSERT(config->signature & (ECS_CMP_MATERIAL));
            break;
            case ECS_CMP_INPUT:
                ASSERT(config->signature & (ECS_CMP_TRANSFORM));
                if (config->component[ECS_CMP_CAMERA_IDX].camera.mode == ECS_CMP_CAMERA_MODE_ORBIT_FOLLOW) {
                    ASSERT(config->signature & (ECS_CMP_CAMERA));
                }
            break;
            case ECS_CMP_MATERIAL:
                ASSERT(config->signature & (ECS_CMP_TRANSFORM));
            break;
            case ECS_CMP_CAMERA :
                ASSERT(config->signature & (ECS_CMP_TRANSFORM));
            break;
        }
    }
}


void ecs_componentmanager_add(ecs_componentmanager_t * const self, const u32 entity_id, ecs_componentbundle_t config)
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

    ecs_component__internal_bundle_validate_and_initalize_internals(&config);

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

        //NOTE: sets the entity id
        memcpy(buf.raw_data, &entity_id, sizeof(entity_id));
        //NOTE: sets active flag
        *(buf.raw_data + sizeof(u32)) = true;
        //NOTE: sets compnent data
        memcpy(buf.raw_data + ECS_CMP_POOL_HEADER_SIZE , &config.component[cmp_idx_count], cmp_size);

        ecs_component_poolentry_t *const entry = slot_insert(pool, pool->len, buf.raw_data, ECS_CMP_POOL_HEADER_SIZE + cmp_size);

        switch(cmp_type)
        {
            case ECS_CMP_COLLIDER:
                colliderbatchqueue_add(&self->internal.colliderbatch, (ecs_component_collider_t *)entry->entity_cmpdata);
            break;
        }
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
    void * const pooldata = (u8 *)data + ECS_CMP_POOL_HEADER_SIZE;
    return pooldata;
}

void ecs_componentmanager_remove(ecs_componentmanager_t * const self, const u32 entity_id, const ecs_component_type type)
{
    const u32 cmp_idx           = get_index_from_bitflag(type);
    slot_t *const componentpool = slot_get_value(&self->componentpool_slots, cmp_idx);

    if (!hashtable_has_key(&self->entity2components_lookup, (hashtable_key_t){ .u32 = entity_id })) return;

    i16 *cmp_idx_buffer = (i16 *)hashtable_get_value(
        &self->entity2components_lookup, 
        (hashtable_key_t){ .u32 = entity_id }
    );

    const u16 index_of_cmp_to_remove                = cmp_idx_buffer[cmp_idx];
    const bool is_last_element                      = index_of_cmp_to_remove == (componentpool->len - 1);
    cmp_idx_buffer[cmp_idx]                         = ECS_CMP_INVALID_IDX;
    ecs_component_poolentry_t *entry                = slot_get_value(componentpool, index_of_cmp_to_remove);

    ecs_componentmanager__internal_cmp_cleanup(entry, type);
    slot_delete(componentpool, index_of_cmp_to_remove);

    if (is_last_element) {
        return;
    }

    //NOTE: swaps last item to deleted item's index

    const u32 last_element_data_idx                 = componentpool->len - 1;
    ecs_component_poolentry_t *last_element_data    = slot_get_value(componentpool, last_element_data_idx);
    const u32 moved_entity_id                       = ecs_componentmanager__internal_get_entity_id_from_pooldata(last_element_data);
    i16 *const cmp_buf                              = hashtable_get_value(&self->entity2components_lookup, (hashtable_key_t){ .u32 = moved_entity_id });
    cmp_buf[get_index_from_bitflag(type)]           = index_of_cmp_to_remove;
    hashtable_insert(&self->entity2components_lookup, (hashtable_key_t){ .u32 = moved_entity_id }, cmp_buf);

    const u32 full_allocation_size = ECS_CMP_POOL_HEADER_SIZE + ecs_component__internal_get_componenttype_size(type);
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

ecs_entity_query_t ecs_componentmanager__internal_query_components(const ecs_componentmanager_t * const self, const u32 entity_id, const u32 component_signature)
{
    ASSERT(self);
    ASSERT(entity_id >= 0);
    ASSERT(component_signature > 0);

    ecs_entity_query_t result = {0};

    const i16 *const cmp_idx_buffer = hashtable_get_value(&self->entity2components_lookup, (hashtable_key_t){ .u32 = entity_id });

    for (u16 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        if (!(component_signature & (1 << cmp_idx)))
            continue;

        if (cmp_idx_buffer[cmp_idx] == ECS_CMP_INVALID_IDX) 
            eprint("Trying to query unavailble component type idx `%i` from entity id `%i`", cmp_idx, entity_id);


        const slot_t *const pool = slot_get_value(&self->componentpool_slots, cmp_idx);
        result.entity_cmp_data[cmp_idx] = ecs_componentmanager__internal_get_cmpdata_from_pooldata(
            slot_get_value(pool, cmp_idx_buffer[cmp_idx])
        );
    }

    return result;
}

void ecs_componentmanager__internal_update_cmpdata(const ecs_cmp_patch_payload_t request, const ecs_component_type cmp_type, const ecs_component_poolentry_t *entry)
{
    switch(cmp_type)
    {
        case ECS_CMP_COLLIDER: {
            const ecs_component_collider_t *const collider = (ecs_component_collider_t *)entry->entity_cmpdata;
            if (!request.is_active)     JPH_BodyInterface_RemoveBody(global_physics_sys_jolt_instance->bodyinterface, collider->internal.body_id);
            else                        JPH_BodyInterface_AddBody(global_physics_sys_jolt_instance->bodyinterface, collider->internal.body_id, JPH_Activation_Activate);
        } break;
    }
}

void ecs_componentmanager_patch_entity_components(ecs_componentmanager_t *const self, const u32 entity_id, const ecs_cmp_patch_payload_t request)
{
    ASSERT(self);
    ASSERT(entity_id >= 0);
    ASSERT(request.signature >= 0);

    const i16 *const cmp_idx_buffer = hashtable_get_value(&self->entity2components_lookup, (hashtable_key_t){ .u32 = entity_id });

    for (u16 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        if (!(request.signature & (1 << cmp_idx)))
            continue;

        if (cmp_idx_buffer[cmp_idx] == ECS_CMP_INVALID_IDX) 
            eprint("Trying to query unavailble component type idx `%i` from entity id `%i`", cmp_idx, entity_id);


        const slot_t *const pool = slot_get_value(&self->componentpool_slots, cmp_idx);

        ecs_component_poolentry_t *const entry = slot_get_value(pool, cmp_idx_buffer[cmp_idx]);
        ASSERT(entry);

        switch(request.patch_type)
        {
            case ECS_PATCH_CMP_ACTIVE_FIELD:
                entry->is_active = request.is_active;
            break;
            default: eprint("invalid patch request type");
        }

        ecs_componentmanager__internal_update_cmpdata(request, 1 << cmp_idx, entry);
    }
}

void ecs_componentmanager_update(ecs_componentmanager_t *const self)
{
    colliderbatchqueue_upload_to_jolt(&self->internal.colliderbatch);
}

void ecs_componentmanager__internal_cmp_cleanup(const ecs_component_type type, const ecs_component_poolentry_t *poolentry)
{
    switch(type)
    {
        case ECS_CMP_COLLIDER: {
            const ecs_component_collider_t *collider = (ecs_component_collider_t *)poolentry->entity_cmpdata;
            if (collider->internal.body_id)                 JPH_BodyInterface_RemoveAndDestroyBody(global_physics_sys_jolt_instance->bodyinterface, collider->internal.body_id);
            else if (collider->internal.kinematic_body)     JPH_CharacterBase_Destroy((JPH_CharacterBase *)collider->internal.kinematic_body);
        } break;
    }
}

#endif

