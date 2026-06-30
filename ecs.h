#pragma once
#include <poglib/basic.h>
#include "./ecs/entity.h"
#include "./ecs/component.h"
#include "poglib/application.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component/types.h"
#include "poglib/ecs/serialization.h"
#include "poglib/poggen.h"
#include "poglib/util/assetmanager.h"


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
void                ecs_load_savefile(ecs_t *const self, const str_t filepath);

void            ecs_update(ecs_t *const self);
void            ecs_destroy(ecs_t * const self);

#ifndef IGNORE_ECS_IMPLEMENTATION

ecs_t * ecs_init(void)
{
    ASSERT(!global_ecs);

    //FIXME: WTF it needs 500 MB ??
    //TODO: Figure out a way to visualize know where memory is distributed in the system 
    arena_t arena = arena_init(NULL, 500 * MB);
    ecs_t result = {
        .internal = {
            .entity_generator_counter = ECS_ENTITY_INVALID_ID,
            .active_camera = NULL,
        },
        .managers = {
            .entitymanager = ecs_entitymanager(&arena),
            .componentmanager = ecs_componentmanager(&arena),
            .systemmanager = {0},
        }
    };
    result.arena = arena;
    global_ecs = arena_store(&arena, &result, sizeof(ecs_t));
    return global_ecs;
}


glcamera_t * ecs_get_active_camera(ecs_t *const self)
{
    ASSERT(self->internal.active_camera);
    return self->internal.active_camera;
}

void ecs_set_active_camera(ecs_t *const self, const u32 entity_id)
{
    const ecs_entity_query_t view = ecs_entity_query_components(self, entity_id, ECS_CMP_CAMERA);
    ecs_component_camera_t *const camera = view.entity_cmp_data[ECS_CMP_CAMERA_IDX];
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
        .id = ++self->internal.entity_generator_counter,
        .component_signature = component_config.signature,
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

void ecs_entity_remove(ecs_t * const self, const u32 entityId)
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

    const ecs_systemmanager_t * const manager = &self->managers.systemmanager;
    if (!manager->count) return;

    for (u8 idx = 0; idx < ECS_SYSTEM_MAX_COUNT; idx++)
    {
        if (!manager->systems[idx].callback)
            continue;

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

    file_t f = file_init(filepath.data, "w");

    ecs_serializer__internal_write_header(&f);

    slot_iterator(&self->managers.entitymanager.entities, iter)
    {
        const ecs_entity_t *const entity = iter;

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

enum { ECS_LOAD_MAX_ENTITIES = 128, ECS_LOAD_MAX_ASSETS = 32 };

typedef struct {
    u32          asset_id;
    asset_meta_t meta;
    gluniform_registry_t uniforms;
} ecs_load__asset_entry_t;

INTERNAL u32 ecs_load__ensure_asset_loaded(assetmanager_t *const assets, const u32 asset_id, ecs_load__asset_entry_t *const parsed_assets, const u32 parsed_asset_count)
{
    if (hashtable_has_key(&assets->assetmeta_lookup, (hashtable_key_t){ .u32 = asset_id }))
        return asset_id;

    for (u32 i = 0; i < parsed_asset_count; i++)
    {
        if (parsed_assets[i].asset_id == asset_id)
        {
            ecs_load__asset_entry_t *e = &parsed_assets[i];
            u32 new_id = 0;
            switch (e->meta.type)
            {
                case ASSET_TYPE_MODEL:
                    new_id = assetmanager_load_model_async(assets, e->meta.filepath1);
                    break;
                case ASSET_TYPE_GLSL_SHADER:
                    new_id = assetmanager_load_glsl_shader(assets, e->meta.filepath1, e->meta.filepath2, e->uniforms);
                    break;
                case ASSET_TYPE_TEXTURE_SPRITE_ATLAS:
                    new_id = assetmanager_load_spriteatlas(assets, e->meta.filepath1, e->meta.meta.tile_counts.x, e->meta.meta.tile_counts.y);
                    break;
                default: break;
            }
            return new_id ? new_id : asset_id;
        }
    }
    return asset_id;
}

INTERNAL void ecs_load__remap_entity_assets(ecs_componentbundle_t *const bundle, assetmanager_t *const assets, ecs_load__asset_entry_t *const parsed_assets, const u32 parsed_asset_count)
{
    for (u8 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        const ecs_component_type cmp_type = 1 << cmp_idx;
        if ((bundle->signature & cmp_type) == 0) continue;

        switch (cmp_type)
        {
            case ECS_CMP_MODEL: {
                ecs_component_model_t *m = &bundle->component[cmp_idx].model;
                m->asset_id = ecs_load__ensure_asset_loaded(assets, m->asset_id, parsed_assets, parsed_asset_count);
            } break;
            case ECS_CMP_MESH: {
                ecs_component_mesh_t *mesh = &bundle->component[cmp_idx].mesh;
                mesh->asset_id = ecs_load__ensure_asset_loaded(assets, mesh->asset_id, parsed_assets, parsed_asset_count);
            } break;
            case ECS_CMP_MATERIAL: {
                ecs_component_material_t *mat = &bundle->component[cmp_idx].material;
                mat->texture_asset_id = ecs_load__ensure_asset_loaded(assets, mat->texture_asset_id, parsed_assets, parsed_asset_count);
                mat->shader_asset_id  = ecs_load__ensure_asset_loaded(assets, mat->shader_asset_id,  parsed_assets, parsed_asset_count);
            } break;
        }
    }
}

void ecs_load_savefile(ecs_t *const self, const str_t filepath)
{
    if (self->internal.entity_generator_counter) eprint("Clear existing ecs before loading a save file");
    ASSERT(self);
    ASSERT(filepath.data);
    ASSERT(filepath.len > 0);

    if (!file_check_exist(filepath.data))
    {
        eprint("save file not found: `%s`", filepath.data);
        return;
    }

    file_t f = file_init(filepath.data, "r");

    buffer(WORD) line = {0};

    file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));
    if (strncmp((char *)line.raw_data, ECS_SAVE_FILE_HEADER_PREFIX, ECS_SAVE_FILE_HEADER_PREFIX_LEN) != 0)
    {
        eprint("Invalid save file format");
        file_destroy(&f);
        return;
    }

    u32                      max_entity_id       = 0;
    bool                     has_pending_bundle  = false;
    ecs_componentbundle_t    pending_bundle      = {0};
    ecs_component_type       current_cmp         = 0;
    u8                       current_cmp_idx     = 0;

    ecs_componentbundle_t    entity_bundles[ECS_LOAD_MAX_ENTITIES] = {0};
    u32                      entity_count                         = 0;

    ecs_load__asset_entry_t  parsed_assets[ECS_LOAD_MAX_ASSETS] = {0};
    u32                      parsed_asset_count                 = 0;

    while (true)
    {
        memset(line.raw_data, 0, sizeof(line.raw_data));
        file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));

        if (line.raw_data[0] == '\0') continue;

        if (strncmp((char *)line.raw_data, "fin", 3) == 0) break;

        if (strncmp((char *)line.raw_data, "entity:", 7) == 0)
        {
            if (has_pending_bundle)
            {
                if (entity_count < ECS_LOAD_MAX_ENTITIES)
                    entity_bundles[entity_count++] = pending_bundle;
                has_pending_bundle = false;
            }

            u32 entity_id;
            sscanf((char *)line.raw_data, "entity:%u", &entity_id);
            if (entity_id > max_entity_id) max_entity_id = entity_id;

            memset(line.raw_data, 0, sizeof(line.raw_data));
            file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));

            u32 signature = 0;
            sscanf((char *)line.raw_data, "component_signature:%u", &signature);

            pending_bundle.signature = signature;
            memset(pending_bundle.component, 0, sizeof(pending_bundle.component));
            has_pending_bundle = true;
            current_cmp = 0;
            current_cmp_idx = 0;
            continue;
        }

        if (strncmp((char *)line.raw_data, "assetid:", 8) == 0)
        {
            if (has_pending_bundle)
            {
                if (entity_count < ECS_LOAD_MAX_ENTITIES)
                    entity_bundles[entity_count++] = pending_bundle;
                has_pending_bundle = false;
            }

            while (strncmp((char *)line.raw_data, "fin", 3) != 0)
            {
                if (strncmp((char *)line.raw_data, "assetid:", 8) != 0)
                {
                    memset(line.raw_data, 0, sizeof(line.raw_data));
                    file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));
                    continue;
                }

                if (parsed_asset_count >= ECS_LOAD_MAX_ASSETS) break;

                ecs_load__asset_entry_t *entry = &parsed_assets[parsed_asset_count];
                asset_meta_t            *meta  = &entry->meta;
                u32 type_val = 0;
                sscanf((char *)line.raw_data, "assetid:%u,assettype:%u,", &entry->asset_id, &type_val);
                meta->type = (asset_type)type_val;

                const char *path_part = strstr((char *)line.raw_data, "assetpath:");
                if (!path_part) break;
                path_part += 10;

                if (meta->type == ASSET_TYPE_GLSL_SHADER)
                {
                    char buf1[256] = {0}, buf2[256] = {0};
                    sscanf(path_part, "[%255[^,],%255[^]]]", buf1, buf2);
                    meta->filepath1 = str_init(&self->arena, buf1);
                    meta->filepath2 = str_init(&self->arena, buf2);

                    while (true)
                    {
                        memset(line.raw_data, 0, sizeof(line.raw_data));
                        file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));
                        if (strncmp((char *)line.raw_data, "\tuniform:", 9) == 0)
                        {
                            memset(line.raw_data, 0, sizeof(line.raw_data));
                            file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));
                            char name[128] = {0};
                            u32  utype     = 0;
                            if (sscanf((char *)line.raw_data, "\t\tname:%127[^,],type:%u", name, &utype) == 2
                                && entry->uniforms.count < MAX_UNIFORMS_ALLOWED_IN_SHADER)
                            {
                                entry->uniforms.data[entry->uniforms.count++] = (gluniform_meta_t){
                                    .name = str_init(&self->arena, name),
                                    .type = (gluniform_type)utype,
                                };
                            }
                            continue;
                        }
                        break;
                    }
                }
                else if (meta->type == ASSET_TYPE_TEXTURE_SPRITE_ATLAS)
                {
                    char buf[256] = {0};
                    sscanf(path_part, "%255s", buf);
                    meta->filepath1 = str_init(&self->arena, buf);
                    meta->meta.tile_counts = (vec2i_t){1, 1};

                    while (true)
                    {
                        memset(line.raw_data, 0, sizeof(line.raw_data));
                        file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));
                        if (strncmp((char *)line.raw_data, "\ttilecount:[", 12) == 0)
                        {
                            sscanf((char *)line.raw_data, "\ttilecount:[%u,%u]", &meta->meta.tile_counts.x, &meta->meta.tile_counts.y);
                            continue;
                        }
                        break;
                    }
                }
                else
                {
                    char buf[256] = {0};
                    sscanf(path_part, "%255s", buf);
                    meta->filepath1 = str_init(&self->arena, buf);

                    memset(line.raw_data, 0, sizeof(line.raw_data));
                    file_readline(&f, (char *)line.raw_data, sizeof(line.raw_data));
                }
                parsed_asset_count++;
            }

            break;
        }

        for (u8 i = 0; i < ECS_CMP_COUNT; i++)
        {
            if (strncmp((char *)line.raw_data, ecs_deserializer__internal_cmp_prefix_map[i].prefix.data, ecs_deserializer__internal_cmp_prefix_map[i].prefix.len) == 0)
            {
                current_cmp     = ecs_deserializer__internal_cmp_prefix_map[i].type;
                current_cmp_idx = ecs_deserializer__internal_cmp_prefix_map[i].idx;
                break;
            }
        }
        if (current_cmp) continue;

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
        if (entity_count < ECS_LOAD_MAX_ENTITIES)
            entity_bundles[entity_count++] = pending_bundle;
    }

    assetmanager_t *assets = &global_engine->systems.assets;

    for (u32 i = 0; i < entity_count; i++)
    {
        ecs_load__remap_entity_assets(&entity_bundles[i], assets, parsed_assets, parsed_asset_count);
        ecs_entity_add(self, entity_bundles[i]);
    }

    self->internal.entity_generator_counter = max_entity_id;

    file_destroy(&f);
}

#endif
