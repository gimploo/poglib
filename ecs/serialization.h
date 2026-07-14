#pragma once
#include "poglib/ecs/component/types.h"
#include "poglib/ecs/serializers.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/poggen.h"
#include "poglib/util/asset.h"

#define ECS_VERSION  "v2"

typedef enum ECS_SERIALIZER_SECTION_TYPES {

    ECS_SERIALIZER_SECTION_HEADER = 0,
    ECS_SERIALIZER_SECTION_ASSETS,
    ECS_SERIALIZER_SECTION_ENTITIES,
    ECS_SERIALIZER_SECTION_UNIFORMS,
    ECS_SERIALIZER_SECTION_COUNT,

} ECS_SERIALIZER_SECTION_TYPES;

const struct {

    const str_t begin;
    const str_t end;

} ECS_SERIALIZER_SECTIONS[] = {

    [ECS_SERIALIZER_SECTION_HEADER] = {
        .begin  = str_lit("section_begin:header"),
        .end    = str_lit("section_end:header")
    },
    [ECS_SERIALIZER_SECTION_ASSETS] = {
        .begin  = str_lit("section_begin:assets"),
        .end    = str_lit("section_end:assets")
    },
    [ECS_SERIALIZER_SECTION_ENTITIES] = {
        .begin  = str_lit("section_begin:entities"),
        .end    = str_lit("section_end:entities")
    },
    [ECS_SERIALIZER_SECTION_UNIFORMS] = {
        .begin  = str_lit("\t\tsection_begin:uniforms"),
        .end    = str_lit("\t\tsection_end:uniforms")
    }
};

INTERNAL gluniform_registry_t ecs_serializer__internal__get_uniform(ecs_t *const self, const str_views_t lines, u64 *const line_idx, arena_t *arena)
{
    gluniform_registry_t registry = {0};

    *line_idx += 1;
    if (!str_cmp(str_lstrip(lines.views[*line_idx], '\t'), str_lstrip(ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_UNIFORMS].begin, '\t')))
        eprint("uniforms start region not found");

    while (true)
    {
        *line_idx += 1;
        const str_t line = str_lstrip(lines.views[*line_idx], '\t');
        if (str_cmp(line, str_lstrip(ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_UNIFORMS].end, '\t')))
            break;

        const str_views_t views = str_split(line, ',', arena);
        ASSERT(views.count == 2);

        const str_t uniform_name = str_partition(views.views[0], ':').pair[1];

        gluniform_type uniform_type;
        sscanf(views.views[1].data, "type:%i", &uniform_type);

        registry.data[registry.count++] = (gluniform_meta_t) {
            .name = str_clone(uniform_name, self->arena),
            .type = uniform_type
        };
    }

    return registry;
}

INTERNAL void ecs_serializer__internal__get_tilecounts(const str_views_t lines, u64 *const line_idx, u32 *const tile_count_width, u32 *const tile_count_height)
{
    const str_t line = str_lstrip(lines.views[*line_idx], '\t');
    const str_pair_t pair = str_partition(line, ':');
    if (!str_cmp(pair.pair[0], str("tilecount"))) {
        eprint("tile count missing");
    }
    sscanf(pair.pair[1].data, "[%i,%i]", tile_count_width, tile_count_height);
}

u32 ecs_serializer_validate_header(const str_views_t lines)
{
    u8 buffer[WORD] = {0};

    if (!str_cmp(lines.views[0], ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_HEADER].begin)) {
        eprint("expected "STR_FMT" not found", STR_ARG(ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_HEADER].begin));
    }
    //NOTE: validate the header.system
    {
        memset(buffer, 0, sizeof(buffer));
        sscanf(str_trim(lines.views[1]).data, "system:%[^\n]", buffer);
        ASSERT(strncmp(buffer, "ecs", strlen(buffer)) == 0);
    }

    //NOTE: validate the header.version
    {
        memset(buffer, 0, sizeof(buffer));
        sscanf(str_trim(lines.views[2]).data, "version:%[^\n]", buffer);
        ASSERT(strncmp(buffer, ECS_VERSION, strlen(buffer)) == 0);
    }

    if (!str_cmp(lines.views[3], ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_HEADER].end)) {
        eprint("expected "STR_FMT" not found", STR_ARG(ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_HEADER].end));
    }

    return 4;

}

const struct {

    void (*serializer)  (file_t *const file, const void *const cmp_data);
    u64 (*deserializer)(const str_views_t lines, const u64 line_index, void *const cmp_data, arena_t *const arena, const hashtable_t *const assetid_remaps);

} ECS_COMPONENT_SERIALIZER_LOOKUP[ECS_CMP_COUNT] = {

    [ECS_CMP_TRANSFORM_IDX] = {
        .serializer     = ecs_transform_serializer,
        .deserializer   = ecs_transform_deserializer,
    },
    [ECS_CMP_MODEL_IDX] = {
        .serializer     = ecs_model_serializer,
        .deserializer   = ecs_model_deserializer,
    },
    [ECS_CMP_INPUT_IDX] = {
        .serializer     = ecs_input_serializer,
        .deserializer   = ecs_input_deserializer,
    },
    [ECS_CMP_MATERIAL_IDX] = {
        .serializer     = ecs_material_serializer,
        .deserializer   = ecs_material_deserializer,
    },
    [ECS_CMP_CAMERA_IDX] = {
        .serializer     = ecs_camera_serializer,
        .deserializer   = ecs_camera_deserializer,
    },
    [ECS_CMP_COLLIDER_IDX] = {
        .serializer     = ecs_collider_serializer,
        .deserializer   = ecs_collider_deserializer,
    },
    [ECS_CMP_MESH_IDX] = {
        .serializer     = ecs_mesh_serializer,
        .deserializer   = ecs_mesh_deserializer,
    }
};


hashtable_t ecs_serializer_load_all_assets(ecs_t *const self, arena_t *arena, const str_views_t lines, u64 *const line_idx)
{
    if (!str_cmp(str_lstrip(lines.views[*line_idx], '\t'), ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_ASSETS].begin)) {
        eprint("asset section begin not found");
    }

    hashtable_t assetid_remaps = hashtable_init(
        30, HT_KEY_TYPE_U32, (ht_value_type){ .type = HT_STORAGE_BY_VALUE_INLINE, .size = sizeof(void *) }, arena);

    while (true)
    {
        *line_idx += 1;

        if (str_cmp(str_lstrip(lines.views[*line_idx], '\t'), ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_ASSETS].end)) {
            *line_idx += 1;
            return assetid_remaps;
        }

        const str_views_t tokens = str_split(
            str_trim(lines.views[*line_idx]), 
            ',',
            arena
        );

        u32 oldasset_id;
        asset_type assettype;
        sscanf(tokens.views[0].data, "assetid:%d", &oldasset_id);
        sscanf(tokens.views[1].data, "assettype:%u", &assettype);

        const str_t asstpath01 = str_clone(str_rstrip(str_partition(tokens.views[2], '[').pair[1], ']'), arena);
        const str_t asstpath02 = tokens.count == 4 
            ? str_clone(str_rstrip(tokens.views[3], ']'), arena)
            : STR_EMPTY;

        u32 tile_count_width                        = 0; 
        u32 tile_count_height                       = 0;
        gluniform_registry_t gluniform_registry     = {0};

        switch(assettype)
        {
            case ASSET_TYPE_GLSL_SHADER:
                gluniform_registry = ecs_serializer__internal__get_uniform(self, lines, line_idx, arena);
            break;
            case ASSET_TYPE_TEXTURE_SPRITE_ATLAS:
                *line_idx += 1;
                ecs_serializer__internal__get_tilecounts(lines, line_idx, &tile_count_width, &tile_count_height);
            break;
        }

        //NOTE: currently the workbench uses the assetmanager and if there are entities that uses some of the mesh primitives will need to 
        //skip it over since those are not loaded from file
        if (assetmanager_does_asset_exist(&global_engine->systems.assets, assettype, oldasset_id)) {
            continue;
        }

        u64 new_asset_id;
        switch(assettype)
        {
            case ASSET_TYPE_MODEL:
                 new_asset_id = assetmanager_load_model_async(
                    &global_engine->systems.assets,
                    asstpath01
                );
            break;

            case ASSET_TYPE_GLSL_SHADER:
                new_asset_id = assetmanager_load_glsl_shader(
                    &global_engine->systems.assets,
                    asstpath01,
                    asstpath02,
                    gluniform_registry
                );
            break;

            case ASSET_TYPE_TEXTURE:
                new_asset_id = assetmanager_load_texture(
                    &global_engine->systems.assets,
                    asstpath01
                );
            break;

            case ASSET_TYPE_TEXTURE_SPRITE_ATLAS:
                new_asset_id = assetmanager_load_spriteatlas(
                    &global_engine->systems.assets,
                    asstpath01,
                    tile_count_width, 
                    tile_count_height
                );
            break;

            default: eprint("type not accounted for");
        }

        hashtable_insert(&assetid_remaps, (hashtable_key_t) { .u32 = oldasset_id }, new_asset_id);
    }

    return assetid_remaps;
}

INTERNAL void assetmanager__internal_write_uniformlocs_to_file(const hashtable_entry_t *const entry, buffer_t *const buffer)
{
    gluniform_meta_t *uniformmeta = entry->value;
    snprintf(buffer->raw_data, buffer->size, 
        "\t\t\tuniform_name:%.*s,type:%u\n",
        uniformmeta->name.len, uniformmeta->name.data, uniformmeta->type
    );
}

void assetmanager_write_assetmeta_data_to_file(const assetmanager_t *const self, file_t *const file)
{
    ASSERT(!file->is_closed);

    hashtable_iterator(&self->assetmeta_lookup, iter)
    {
        buffer(WORD) buffer = {0};
        const hashtable_entry_t *entry          = iter;
        const asset_meta_t *const assetmeta     = entry->value;

        if (assetmeta->filepath1.len && assetmeta->filepath2.len)
            snprintf(
                buffer.raw_data, sizeof(buffer.raw_data), 
                "\tassetid:%u,assettype:%u,assetpath:[%.*s,%.*s]\n",
                entry->key.u32,
                assetmeta->type,
                assetmeta->filepath1.len,
                assetmeta->filepath1.data,
                assetmeta->filepath2.len,
                assetmeta->filepath2.data
            );
        else if (!assetmeta->filepath1.len && assetmeta->filepath2.len)
            snprintf(
                buffer.raw_data, sizeof(buffer.raw_data), 
                "\tassetid:%u,assettype:%u,assetpath:[%.*s]\n",
                entry->key.u32,
                assetmeta->type,
                assetmeta->filepath2.len,
                assetmeta->filepath2.data
            );
        else if (assetmeta->filepath1.len && !assetmeta->filepath2.len)
            snprintf(
                buffer.raw_data, sizeof(buffer.raw_data), 
                "\tassetid:%u,assettype:%u,assetpath:[%.*s]\n",
                entry->key.u32,
                assetmeta->type,
                assetmeta->filepath1.len,
                assetmeta->filepath1.data
            );

        file_writeline(file, buffer.raw_data);
        memset(buffer.raw_data, 0, sizeof(buffer.raw_data));

        switch(assetmeta->type)
        {
            case ASSET_TYPE_TEXTURE_SPRITE_ATLAS:
                snprintf(
                    buffer.raw_data, sizeof(buffer.raw_data), 
                    "\t\ttilecount:[%u,%u]\n",
                    assetmeta->meta.tile_counts.x,
                    assetmeta->meta.tile_counts.y
                );
            break;
            case ASSET_TYPE_GLSL_SHADER:
                file_writeline(file, ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_UNIFORMS].begin.data);
                file_writeline(file, "\n");
                hashtable_serialize_to_file(
                    assetmeta->meta.uniformlocs, 
                    file,
                    assetmanager__internal_write_uniformlocs_to_file
                );
                file_writeline(file, ECS_SERIALIZER_SECTIONS[ECS_SERIALIZER_SECTION_UNIFORMS].end.data);
                file_writeline(file, "\n");
            break;

        }
        file_writeline(file, buffer.raw_data);
    }
}



