#pragma once
#include <poglib/basic.h>
#include "./ecs/entity.h"
#include "./ecs/component.h"
#include "poglib/application.h"
#include "poglib/basic/arena.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component/types.h"
#include "poglib/ecs/serialization.h"
#include "poglib/util/assetmanager.h"
#include "poglib/poggen.h"


ecs_t * global_ecs = NULL;

ecs_t *         ecs_init(void);

u32                 ecs_entity_add(ecs_t * const self, const ecs_componentbundle_t component_config);
u32                 ecs_entity_duplicate(ecs_t *const self, const u32 entity_id);
void                ecs_entity_remove(ecs_t * const self, const u32 entityId);

ecs_entity_query_t  ecs_entity_query_components(ecs_t *const self, const u32 entity_id, const u32 component_signature);

void                ecs_set_active_camera(ecs_t *const self, const u32 entity_id);
glcamera_t *        ecs_get_active_camera(ecs_t *const self);

void                ecs_patch_entity(ecs_t *const self, const u32 entity_id, const ecs_cmp_patch_payload_t request);

void                ecs_save_to_file(ecs_t *const self, const str_t filepath);
bool                ecs_load_savefile(ecs_t *const self, const str_t filepath);

void            ecs_update(ecs_t *const self);
void            ecs_destroy(ecs_t *const self);

#include "poglib/ecs/system.h"

#ifndef IGNORE_ECS_IMPLEMENTATION

ecs_t * ecs_init(void)
{
    ASSERT(!global_ecs);

    //FIXME: WTF it needs 500 MB ??
    //TODO: Figure out a way to visualize know where memory is distributed in the system 
    arena_t arena   = arena_init(NULL, 500 * MB);
    global_ecs      = arena_reserve(&arena, sizeof(ecs_t));

    *global_ecs = (ecs_t){
        .internal = {
            .entity_generator_counter = ECS_ENTITY_INVALID_ID,
            .active_camera = NULL,
        },
        .managers = {
            .entitymanager      = ecs_entitymanager(&arena),
            .componentmanager   = ecs_componentmanager(&arena),
            .systemmanager      = {0},
        },
        .arena = arena
    };

    ecs_add_all_core_systems(global_ecs);
    return global_ecs;
}


glcamera_t * ecs_get_active_camera(ecs_t *const self)
{
    ASSERT(self->internal.active_camera);
    return self->internal.active_camera;
}

void ecs_set_active_camera(ecs_t *const self, const u32 entity_id)
{
    const ecs_entity_query_t view           = ecs_entity_query_components(self, entity_id, ECS_CMP_CAMERA);
    ecs_component_camera_t *const camera    = view.entity_cmp_data[ECS_CMP_CAMERA_IDX];
    ASSERT(camera);

    self->internal.active_camera = &camera->camera;
}

void ecs_patch_entity(ecs_t *const self, const u32 entity_id, const ecs_cmp_patch_payload_t request)
{
    ecs_componentmanager_patch_entity_components(
        &self->managers.componentmanager, 
        entity_id,
        request
    );
}

u32 ecs_entity_add(ecs_t *const self, const ecs_componentbundle_t component_config)
{
    const ecs_entity_t new_entity = {
        .id                     = ++self->internal.entity_generator_counter,
        .component_signature    = component_config.signature,
    };

    ecs_entitymanager_add(
        &self->managers.entitymanager, 
        new_entity
    );

    ecs_componentmanager_add(
        &self->managers.componentmanager,
        new_entity.id,
        component_config
    );
    return new_entity.id;
}

u32 ecs_entity_duplicate(ecs_t *const self, const u32 entity_id)
{
    const u64 entity_idx = (u64)hashtable_get_value(
        &self->managers.entitymanager.entityid_to_entityidx_lookup, (hashtable_key_t) {
            .u32 = entity_id
        });

    ecs_componentbundle_t component_config = ecs_componentmanager_get_componentbundle_from_existing_entity(
        &self->managers.componentmanager, 
        *(ecs_entity_t *)slot_get_value(&self->managers.entitymanager.entities, entity_idx)
    );

    return ecs_entity_add(self, component_config);
}

void ecs_entity_remove(ecs_t *const self, const u32 entityId)
{
    ecs_entitymanager_remove(&self->managers.entitymanager, entityId);
    ecs_componentmanager_removeall(
        &self->managers.componentmanager,
        entityId
    );
}

void ecs_update(ecs_t *const self)
{
    ASSERT(self);

    ecs_componentmanager_update(&self->managers.componentmanager);

    const ecs_systemmanager_t *const manager = &self->managers.systemmanager;
    if (!manager->count) return;

    for (u8 idx = 0; idx < ECS_SYSTEM_MAX_COUNT; idx++)
    {
        if (!manager->systems[idx].callback) continue;

        manager->systems[idx].callback(
            &self->managers.componentmanager,
            (ecs_system_ctx_t) {
                .active_camera = self->internal.active_camera,
                .dt = APPLICATION_UPDATE_FIXED_TIME_STEP
            }
        );
    }
}

ecs_entity_query_t ecs_entity_query_components(ecs_t *const self, const u32 entity_id, const u32 component_signature)
{
    return ecs_componentmanager__internal_query_components(&self->managers.componentmanager, entity_id, component_signature);
}

void ecs_destroy(ecs_t * const self)
{
    arena_destroy(&self->arena);
}

void ecs_save_to_file(ecs_t *const self, const str_t filepath)
{
    ASSERT(self);
    ASSERT(global_engine);

    file_t f = file_init(filepath.data, "w");

    ecs_serializer__internal_write_header(&f);

    slot_iterator(&self->managers.entitymanager.entities, iter)
    {
        const ecs_entity_t *const entity = iter;

        if (entity->id < WORKBENCH_RESERVED_ENTITY_ID_COUNT) continue;

        ecs_serializer__internal_write_entity_data(&f, entity->id, entity->component_signature);

        ecs_entity_query_t query = ecs_entity_query_components(
            self, entity->id, entity->component_signature
        );

        for (u8 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
        {
            const ecs_component_type cmp_type = 1 << cmp_idx;
            if ((entity->component_signature & cmp_type) == 0) continue;

            ecs_serializer__internal_entity_cmp_data(&f, cmp_type, query.entity_cmp_data[cmp_idx]);
        }
    }

    assetmanager_write_assetmeta_data_to_file(&global_engine->systems.assets, &f);

    ecs_serializer__internal_write_footer(&f);

    file_destroy(&f);
}

bool ecs_load_savefile(ecs_t *const self, const str_t filepath)
{
    if (self->internal.entity_generator_counter >= WORKBENCH_RESERVED_ENTITY_ID_COUNT) 
        eprint("Clear scene's ecs before loading a save file");

    ASSERT(self);
    ASSERT(filepath.data);
    ASSERT(filepath.len > 0);
    ASSERT(global_engine);

    if (!file_check_exist(filepath.data))
    {
        //eprint("save file not found: `%s`", filepath.data);
        return false;
    }

    file_t f = file_init(filepath.data, "r");

    buffer(WORD) line = {0};

    file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));
    if (strncmp((char *)line.raw_data, ECS_SAVE_FILE_HEADER_PREFIX.data, ECS_SAVE_FILE_HEADER_PREFIX.len) != 0)
    {
        eprint("Invalid save file format");
        file_destroy(&f);
        return false;
    }

    u32                      max_entity_id       = 0;
    bool                     has_pending_bundle  = false;
    ecs_componentbundle_t    pending_bundle      = {0};
    ecs_component_type       current_cmp         = 0;
    u8                       current_cmp_idx     = 0;

    ecs_load__entity_list_t *const entities      = arena_reserve(&self->arena, sizeof(ecs_load__entity_list_t));
    ecs_load__asset_list_t  *const assets_parsed = arena_reserve(&self->arena, sizeof(ecs_load__asset_list_t));

    while (true)
    {
        memset(line.raw_data, 0, sizeof(line.raw_data));
        file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));

        if (line.raw_data[0] == '\0') 
            continue;

        if (strncmp((char *)line.raw_data, ECS_DESERIALIZER_FIN.data, ECS_DESERIALIZER_FIN.len) == 0) 
            break;

        if (strncmp((char *)line.raw_data, ECS_DESERIALIZER_ENTITY_PREFIX.data, ECS_DESERIALIZER_ENTITY_PREFIX.len) == 0)
        {
            if (has_pending_bundle)
            {
                if (entities->count < ECS_LOAD_MAX_ENTITIES) {
                    entities->data[entities->count++] = pending_bundle;
                }
                has_pending_bundle = false;
            }

            u32 entity_id;
            sscanf((char *)line.raw_data, "entity:%u", &entity_id);
            if (entity_id > max_entity_id) {
                max_entity_id = entity_id;
            }

            memset(line.raw_data, 0, sizeof(line.raw_data));
            file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));

            u32 signature = 0;
            sscanf((char *)line.raw_data, "component_signature:%u", &signature);

            pending_bundle.signature = signature;
            memset(pending_bundle.component, 0, sizeof(pending_bundle.component));

            has_pending_bundle  = true;
            current_cmp         = 0;
            current_cmp_idx     = 0;
            continue;
        }

        if (strncmp((char *)line.raw_data, ECS_DESERIALIZER_ASSETID_PREFIX.data, ECS_DESERIALIZER_ASSETID_PREFIX.len) == 0)
        {
            if (has_pending_bundle)
            {
                if (entities->count < ECS_LOAD_MAX_ENTITIES)
                    entities->data[entities->count++] = pending_bundle;
                has_pending_bundle = false;
            }

            ecs_deserializer__internal__read_assetmeta_section(&f, assets_parsed, &self->arena, (char *)line.raw_data, sizeof(line.raw_data));

            break;
        }

        bool prefix_matched = false;
        for (u8 i = 0; i < ECS_CMP_COUNT; i++)
        {
            if (strncmp((char *)line.raw_data, ECS_DESERIALIZER__INTERNAL_CMP_PREFIX_MAP[i].prefix.data, ECS_DESERIALIZER__INTERNAL_CMP_PREFIX_MAP[i].prefix.len) == 0)
            {
                current_cmp     = ECS_DESERIALIZER__INTERNAL_CMP_PREFIX_MAP[i].type;
                current_cmp_idx = ECS_DESERIALIZER__INTERNAL_CMP_PREFIX_MAP[i].idx;
                prefix_matched  = true;
                break;
            }
        }
        if (prefix_matched) continue;

        if (line.raw_data[0] == '\t' && has_pending_bundle)
        {
            ecs_deserializer__internal_parse_cmp_data_line(
                (char *)line.raw_data,
                current_cmp,
                &pending_bundle.component[current_cmp_idx]
            );
        }
    }

    if (has_pending_bundle)
    {
        if (entities->count < ECS_LOAD_MAX_ENTITIES)
            entities->data[entities->count++] = pending_bundle;
    }

    for (u32 i = 0; i < entities->count; i++)
    {
        ecs_deserializer__internal_remap_entity_assets(&entities->data[i], &global_engine->systems.assets, assets_parsed);
        ecs_entity_add(self, entities->data[i]);
    }

    file_destroy(&f);
    return true;
}

#endif

