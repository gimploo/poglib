#pragma once
#include "./common.h"
#include "poglib/ecs/component/types.h"
#include "poglib/util/assetmanager.h"

INTERNAL const str_t ECS_DESERIALIZER_ENTITY_PREFIX        = str_lit("entity:");
INTERNAL const str_t ECS_DESERIALIZER_SIGNATURE_PREFIX    = str_lit("component_signature:");
INTERNAL const str_t ECS_DESERIALIZER_ASSETID_PREFIX      = str_lit("assetid:");
INTERNAL const str_t ECS_DESERIALIZER_FIN                 = str_lit("fin");

INTERNAL const struct {
    ecs_component_type type;
    u8 idx;
    str_t prefix;
} ecs_deserializer__internal_cmp_prefix_map[ECS_CMP_COUNT] = {
    { ECS_CMP_TRANSFORM, ECS_CMP_TRANSFORM_IDX, str_lit("transform:") },
    { ECS_CMP_MODEL,     ECS_CMP_MODEL_IDX,     str_lit("model:")     },
    { ECS_CMP_INPUT,     ECS_CMP_INPUT_IDX,     str_lit("input:")     },
    { ECS_CMP_MATERIAL,  ECS_CMP_MATERIAL_IDX,  str_lit("material:")  },
    { ECS_CMP_CAMERA,    ECS_CMP_CAMERA_IDX,    str_lit("camera:")    },
    { ECS_CMP_COLLIDER,  ECS_CMP_COLLIDER_IDX,  str_lit("collider:")  },
    { ECS_CMP_MESH,      ECS_CMP_MESH_IDX,      str_lit("mesh:")      },
};

INTERNAL void ecs_serializer__internal_write_header(file_t *const file)
{
    const u32 magic        = ECS_SAVE_FILE_MAGIC;
    const u32 version      = ECS_SAVE_FILE_VERSION;

    buffer(WORD) buffer = {0};
    snprintf(buffer.raw_data, sizeof(buffer.raw_data), "ECS v%i save\n", version);
    file_writeline(file, (char *)buffer.raw_data);
}

INTERNAL void ecs_serializer__internal_write_entity_data(file_t *const file, const u32 entity_id, const u32 signature)
{
    const char *format = ""
    "entity:%lu\n"
    "component_signature:%lu\n";

    buffer(WORD) buffer = {0};
    snprintf(buffer.raw_data, sizeof(buffer.raw_data), format, entity_id, signature);
    file_writeline(file, (char *)buffer.raw_data);
}

INTERNAL void ecs_serializer__internal_entity_cmp_data(file_t *const file, ecs_component_type type, void *cmp_data)
{
    buffer(WORD) buffer = {0};

    switch(type)
    {
        case ECS_CMP_TRANSFORM: {
            const ecs_component_transform_t *const transform = cmp_data;
            snprintf(
                buffer.raw_data, 
                sizeof(buffer.raw_data),

                "transform:{totalmembers:%lu,bytesize:%lu}\n"
                "\tposition:[%f,%f,%f]\n"
                "\torientation:[%f,%f,%f,%f]\n"
                "\tscale:[%f,%f,%f]\n"
                "\tvelocity:[%f,%f,%f]\n"
                "\tsource:%i\n",

                5, sizeof(ecs_component_transform_t),
                transform->position.x, transform->position.y, transform->position.z,
                transform->orientation.x, transform->orientation.y, transform->orientation.z, transform->orientation.w,
                transform->scale.x, transform->scale.y, transform->scale.z,
                transform->velocity.x, transform->velocity.y, transform->velocity.z,
                transform->source
            );
        } break;
        case ECS_CMP_MODEL: {
            const ecs_component_model_t *const model = cmp_data;
            snprintf(
                buffer.raw_data, 
                sizeof(buffer.raw_data),

                "model:{totalmembers:%lu,bytesize:%lu}\n"
                "\tasset_id:%lu\n",

                2, sizeof(ecs_component_model_t),
                model->asset_id
            );
        } break;
        case ECS_CMP_INPUT: {
            const ecs_component_input_t *const input = cmp_data;
            snprintf(
                buffer.raw_data,
                sizeof(buffer.raw_data),

                "input:{totalmembers:%lu,bytesize:%lu}\n"
                "\tdirection_source:%i\n"
                "\tinput_behavior:%p\n"
                "\tstate.position:[%f,%f,%f]\n"
                "\tstate.orientation:[%f,%f,%f,%f]\n"
                "\tstate.front:[%f,%f,%f]\n"
                "\tstate.right:[%f,%f,%f]\n",

                3, sizeof(ecs_component_input_t),
                input->direction_source,
                (void *)input->input_behavior,
                input->internal.state.current_position.x, input->internal.state.current_position.y, input->internal.state.current_position.z,
                input->internal.state.current_orientation.x, input->internal.state.current_orientation.y, input->internal.state.current_orientation.z, input->internal.state.current_orientation.w,
                input->internal.state.front.x, input->internal.state.front.y, input->internal.state.front.z,
                input->internal.state.right.x, input->internal.state.right.y, input->internal.state.right.z
            );
        } break;
        case ECS_CMP_MATERIAL: {
            const ecs_component_material_t *const material = cmp_data;
            snprintf(
                buffer.raw_data, 
                sizeof(buffer.raw_data),
                "material:{totalmembers:%lu,bytesize:%lu}\n"
                "\ttexture_asset_id:%u\n"
                "\tshader_asset_id:%u\n",
                2, sizeof(ecs_component_material_t), material->texture_asset_id, material->shader_asset_id
            );
        } break;
       case ECS_CMP_CAMERA: {
            const ecs_component_camera_t *const camera = cmp_data;
            snprintf(
                buffer.raw_data,
                sizeof(buffer.raw_data),

                "camera:{totalmembers:%lu,bytesize:%lu}\n"
                "\tposition:[%f,%f,%f]\n"
                "\teuler_angle:[%f,%f]\n"
                "\tdirection.front:[%f,%f,%f]\n"
                "\tdirection.up:[%f,%f,%f]\n"
                "\tdirection.right:[%f,%f,%f]\n"
                "\tmode:%i\n"
                "\tfollow.orbit_radius:%f\n"
                "\tfollow.center_offset:[%f,%f,%f]\n"
                "\tfollow.track_entity_id:%u\n",

                3, sizeof(ecs_component_camera_t),

                camera->camera.position.x, camera->camera.position.y, camera->camera.position.z,
                camera->camera.euler_angle.x, camera->camera.euler_angle.y,
                camera->camera.direction.front.x, camera->camera.direction.front.y, camera->camera.direction.front.z,
                camera->camera.direction.up.x, camera->camera.direction.up.y, camera->camera.direction.up.z,
                camera->camera.direction.right.x, camera->camera.direction.right.y, camera->camera.direction.right.z,
                camera->mode,
                camera->follow.orbit_radius,
                camera->follow.center_offset.x, camera->follow.center_offset.y, camera->follow.center_offset.z,
                camera->follow.track_entity_id
            );
        } break;
        case ECS_CMP_COLLIDER: {
            const ecs_component_collider_t *const collider = cmp_data;
            snprintf(
                buffer.raw_data,
                sizeof(buffer.raw_data),

                "collider:{totalmembers:%lu,bytesize:%lu}\n"
                "\tshape_type:%i\n"
                "\tmotion_type:%i\n"
                "\tobject_layer_type:%lu\n"
                "\tdim.cube:[%f,%f,%f]\n"
                "\tdim.sphere:[%f]\n"
                "\tdim.capsule:[%f,%f]\n",

                5, sizeof(ecs_component_collider_t),
                collider->shape_type,
                collider->motion_type,
                (unsigned long)collider->object_layer_type,
                collider->dim.cube.half_width, collider->dim.cube.half_height, collider->dim.cube.half_depth,
                collider->dim.sphere.radius,
                collider->dim.capsule.radius, collider->dim.capsule.half_height
            );
        } break;
        case ECS_CMP_MESH: {
            const ecs_component_mesh_t *const mesh = cmp_data;
            snprintf(
                buffer.raw_data,
                sizeof(buffer.raw_data),

                "mesh:{totalmembers:%lu,bytesize:%lu}\n"
                "\tasset_id:%lu\n"
                "\tprototype_sprite_type:%i\n",

                2, sizeof(ecs_component_mesh_t),
                mesh->asset_id,
                mesh->prototype_sprite_type
            );
        } break;

    }
    file_writeline(file, (char *)buffer.raw_data);
}


void assetmanager_serialize_to_file(const assetmanager_t *const self, file_t *const file)
{
    ASSERT(!file->is_closed);

    buffer(WORD) buffer = {0};
    hashtable_iterator(&self->assetmeta_lookup, iter)
    {
        const hashtable_entry_t *entry  = iter;
        const str_t *const assetpath    = entry->value;
        snprintf(
            buffer.raw_data, buffer.size, 
            "assetid:%i,assetpath:%*s\n",
            entry->key.u32,
            assetpath->len,
            assetpath->data
        );
        file_writeline(file, buffer.raw_data);
    }
}

typedef enum {
    ECS_CMP_FLD_FLOAT3,
    ECS_CMP_FLD_FLOAT4,
    ECS_CMP_FLD_FLOAT1,
    ECS_CMP_FLD_FLOAT2,
    ECS_CMP_FLD_INT1,
    ECS_CMP_FLD_UINT1,
    ECS_CMP_FLD_ULONG1,
    ECS_CMP_FLD_SKIP,
} ecs_cmp_fld_fmt_t;

typedef struct {
    str_t            prefix;
    const char       *scanf_fmt;
    u64              offset;
    ecs_cmp_fld_fmt_t fmt;
} ecs_cmp_field_descriptor_t;

typedef struct {
    u8 field_count;
    ecs_cmp_field_descriptor_t fields[9];
} ecs_cmp_field_map_t;

#define CMP_FLD_DESC(PREFIX, FMT, STRUCT, FIELD, FMT_TYPE) \
    { str_lit(PREFIX), FMT, offsetof(STRUCT, FIELD), FMT_TYPE }

INTERNAL const ecs_cmp_field_map_t ecs_deserializer__internal_cmp_field_maps[ECS_CMP_COUNT] = {
    [ECS_CMP_TRANSFORM_IDX] = {
        .field_count = 5,
        .fields = {
            CMP_FLD_DESC("\tposition:",    "\tposition:[%f,%f,%f]",       ecs_component_transform_t, position,      ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\torientation:", "\torientation:[%f,%f,%f,%f]", ecs_component_transform_t, orientation,   ECS_CMP_FLD_FLOAT4),
            CMP_FLD_DESC("\tscale:",       "\tscale:[%f,%f,%f]",          ecs_component_transform_t, scale,         ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\tvelocity:",    "\tvelocity:[%f,%f,%f]",       ecs_component_transform_t, velocity,      ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\tsource:",      "\tsource:%i",                 ecs_component_transform_t, source,        ECS_CMP_FLD_INT1),
        }
    },
    [ECS_CMP_MODEL_IDX] = {
        .field_count = 1,
        .fields = {
            CMP_FLD_DESC("\tasset_id:", "\tasset_id:%lu", ecs_component_model_t, asset_id, ECS_CMP_FLD_ULONG1),
        }
    },
    [ECS_CMP_INPUT_IDX] = {
        .field_count = 6,
        .fields = {
            CMP_FLD_DESC("\tdirection_source:",     "\tdirection_source:%i",            ecs_component_input_t, direction_source,                                         ECS_CMP_FLD_INT1),
            CMP_FLD_DESC("\tinput_behavior:",       "\tinput_behavior:%p",              ecs_component_input_t, input_behavior,                                           ECS_CMP_FLD_SKIP),
            CMP_FLD_DESC("\tstate.position:",       "\tstate.position:[%f,%f,%f]",     ecs_component_input_t, internal.state.current_position,     ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\tstate.orientation:",    "\tstate.orientation:[%f,%f,%f,%f]",ecs_component_input_t, internal.state.current_orientation, ECS_CMP_FLD_FLOAT4),
            CMP_FLD_DESC("\tstate.front:",          "\tstate.front:[%f,%f,%f]",        ecs_component_input_t, internal.state.front,               ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\tstate.right:",          "\tstate.right:[%f,%f,%f]",        ecs_component_input_t, internal.state.right,               ECS_CMP_FLD_FLOAT3),
        }
    },
    [ECS_CMP_MATERIAL_IDX] = {
        .field_count = 2,
        .fields = {
            CMP_FLD_DESC("\ttexture_asset_id:", "\ttexture_asset_id:%u", ecs_component_material_t, texture_asset_id, ECS_CMP_FLD_UINT1),
            CMP_FLD_DESC("\tshader_asset_id:",  "\tshader_asset_id:%u",  ecs_component_material_t, shader_asset_id,  ECS_CMP_FLD_UINT1),
        }
    },
    [ECS_CMP_CAMERA_IDX] = {
        .field_count = 9,
        .fields = {
            CMP_FLD_DESC("\tposition:",              "\tposition:[%f,%f,%f]",           ecs_component_camera_t, camera.position,          ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\teuler_angle:",           "\teuler_angle:[%f,%f]",           ecs_component_camera_t, camera.euler_angle,       ECS_CMP_FLD_FLOAT2),
            CMP_FLD_DESC("\tdirection.front:",       "\tdirection.front:[%f,%f,%f]",    ecs_component_camera_t, camera.direction.front,   ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\tdirection.up:",          "\tdirection.up:[%f,%f,%f]",       ecs_component_camera_t, camera.direction.up,      ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\tdirection.right:",       "\tdirection.right:[%f,%f,%f]",    ecs_component_camera_t, camera.direction.right,   ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\tmode:",                  "\tmode:%i",                        ecs_component_camera_t, mode,                     ECS_CMP_FLD_INT1),
            CMP_FLD_DESC("\tfollow.orbit_radius:",   "\tfollow.orbit_radius:%f",        ecs_component_camera_t, follow.orbit_radius,      ECS_CMP_FLD_FLOAT1),
            CMP_FLD_DESC("\tfollow.center_offset:",  "\tfollow.center_offset:[%f,%f,%f]",ecs_component_camera_t, follow.center_offset,      ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\tfollow.track_entity_id:","\tfollow.track_entity_id:%u",     ecs_component_camera_t, follow.track_entity_id,   ECS_CMP_FLD_UINT1),
        }
    },
    [ECS_CMP_COLLIDER_IDX] = {
        .field_count = 6,
        .fields = {
            CMP_FLD_DESC("\tshape_type:",        "\tshape_type:%i",           ecs_component_collider_t, shape_type,              ECS_CMP_FLD_INT1),
            CMP_FLD_DESC("\tmotion_type:",       "\tmotion_type:%i",          ecs_component_collider_t, motion_type,             ECS_CMP_FLD_INT1),
            CMP_FLD_DESC("\tobject_layer_type:", "\tobject_layer_type:%lu",   ecs_component_collider_t, object_layer_type,       ECS_CMP_FLD_ULONG1),
            CMP_FLD_DESC("\tdim.cube:",          "\tdim.cube:[%f,%f,%f]",     ecs_component_collider_t, dim.cube.half_width,     ECS_CMP_FLD_FLOAT3),
            CMP_FLD_DESC("\tdim.sphere:",        "\tdim.sphere:[%f]",         ecs_component_collider_t, dim.sphere.radius,       ECS_CMP_FLD_FLOAT1),
            CMP_FLD_DESC("\tdim.capsule:",       "\tdim.capsule:[%f,%f]",     ecs_component_collider_t, dim.capsule.radius,      ECS_CMP_FLD_FLOAT2),
        }
    },
    [ECS_CMP_MESH_IDX] = {
        .field_count = 2,
        .fields = {
            CMP_FLD_DESC("\tasset_id:",              "\tasset_id:%lu",              ecs_component_mesh_t, asset_id,              ECS_CMP_FLD_ULONG1),
            CMP_FLD_DESC("\tprototype_sprite_type:", "\tprototype_sprite_type:%i",  ecs_component_mesh_t, prototype_sprite_type, ECS_CMP_FLD_INT1),
        }
    },
};

INTERNAL void ecs_deserializer__internal_parse_cmp_data_line(
    const char *const line,
    const ecs_component_type type,
    void *const cmp_data)
{
    for (u8 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        if (type != (1 << cmp_idx)) continue;

        const ecs_cmp_field_map_t *map = &ecs_deserializer__internal_cmp_field_maps[cmp_idx];
        for (u8 i = 0; i < map->field_count; i++)
        {
            const ecs_cmp_field_descriptor_t *f = &map->fields[i];
            if (strncmp(line, f->prefix.data, f->prefix.len) != 0) continue;

            if (f->fmt == ECS_CMP_FLD_SKIP) return;

            u8 *dest = (u8 *)cmp_data + f->offset;
            switch (f->fmt)
            {
                case ECS_CMP_FLD_FLOAT3: {
                    vec3f_t *v = (vec3f_t *)dest;
                    sscanf(line, f->scanf_fmt, &v->x, &v->y, &v->z);
                } break;
                case ECS_CMP_FLD_FLOAT4: {
                    versors *q = (versors *)dest;
                    sscanf(line, f->scanf_fmt, &q->x, &q->y, &q->z, &q->w);
                } break;
                case ECS_CMP_FLD_FLOAT1:  sscanf(line, f->scanf_fmt, (f32 *)dest);                          break;
                case ECS_CMP_FLD_FLOAT2: {
                    vec2f_t *v = (vec2f_t *)dest;
                    sscanf(line, f->scanf_fmt, &v->x, &v->y);
                } break;
                case ECS_CMP_FLD_INT1:    sscanf(line, f->scanf_fmt, (i32 *)dest);                           break;
                case ECS_CMP_FLD_UINT1:   sscanf(line, f->scanf_fmt, (u32 *)dest);                           break;
                case ECS_CMP_FLD_ULONG1:  sscanf(line, f->scanf_fmt, (u64 *)dest);                           break;
                default: break;
            }
            return;
        }
    }
}

enum { ECS_LOAD_MAX_ENTITIES = 128, ECS_LOAD_MAX_ASSETS = 32 };

typedef struct {
    u32          asset_id;
    asset_meta_t meta;
} ecs_load__asset_entry_t;

INTERNAL u32 ecs_deserializer__internal_ensure_asset_loaded(assetmanager_t *const assets, const u32 asset_id, ecs_load__asset_entry_t *const parsed_assets, const u32 parsed_asset_count)
{
    if (hashtable_has_key(&assets->assetmeta_lookup, (hashtable_key_t){ .u32 = asset_id }))
    {
        const asset_meta_t *existing = hashtable_get_value(&assets->assetmeta_lookup, (hashtable_key_t){ .u32 = asset_id });
        for (u32 i = 0; i < parsed_asset_count; i++)
        {
            if (parsed_assets[i].asset_id == asset_id)
            {
                if (existing->filepath1.len && !str_cmp(existing->filepath1, parsed_assets[i].meta.filepath1)) return asset_id;
                if (existing->filepath2.len && !str_cmp(existing->filepath2, parsed_assets[i].meta.filepath2)) return asset_id;
            }
        }
        return asset_id;
    }

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
                case ASSET_TYPE_GLSL_SHADER: {
                    const asset_meta_t *existing = hashtable_get_value(&assets->assetmeta_lookup, (hashtable_key_t){ .u32 = asset_id });
                    if (!existing) {
                        eprint("cannot load shader %u without uniform data - load assets before calling ecs_load_savefile", asset_id);
                        return asset_id;
                    }
                } break;
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

INTERNAL void ecs_deserializer__internal_remap_entity_assets(ecs_componentbundle_t *const bundle, assetmanager_t *const assets, ecs_load__asset_entry_t *const parsed_assets, const u32 parsed_asset_count)
{
    for (u8 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        const ecs_component_type cmp_type = 1 << cmp_idx;
        if ((bundle->signature & cmp_type) == 0) continue;

        switch (cmp_type)
        {
            case ECS_CMP_MODEL: {
                ecs_component_model_t *m = &bundle->component[cmp_idx].model;
                m->asset_id = ecs_deserializer__internal_ensure_asset_loaded(assets, m->asset_id, parsed_assets, parsed_asset_count);
            } break;
            case ECS_CMP_MESH: {
                ecs_component_mesh_t *mesh = &bundle->component[cmp_idx].mesh;
                mesh->asset_id = ecs_deserializer__internal_ensure_asset_loaded(assets, mesh->asset_id, parsed_assets, parsed_asset_count);
            } break;
            case ECS_CMP_MATERIAL: {
                ecs_component_material_t *mat = &bundle->component[cmp_idx].material;
                mat->texture_asset_id = ecs_deserializer__internal_ensure_asset_loaded(assets, mat->texture_asset_id, parsed_assets, parsed_asset_count);
                mat->shader_asset_id  = ecs_deserializer__internal_ensure_asset_loaded(assets, mat->shader_asset_id,  parsed_assets, parsed_asset_count);
            } break;
        }
    }
}

INTERNAL void ecs_deserializer__internal_read_assetmeta_section(file_t *const f, ecs_load__asset_entry_t *parsed_assets, u32 *parsed_asset_count, arena_t *arena, char *line_buf, u32 buf_size)
{
    (void)buf_size;

    while (strncmp(line_buf, ECS_DESERIALIZER_FIN.data, ECS_DESERIALIZER_FIN.len) != 0)
    {
        if (strncmp(line_buf, ECS_DESERIALIZER_ASSETID_PREFIX.data, ECS_DESERIALIZER_ASSETID_PREFIX.len) != 0)
        {
            memset(line_buf, 0, buf_size);
            file_readline(f, line_buf, buf_size);
            continue;
        }

        if (*parsed_asset_count >= ECS_LOAD_MAX_ASSETS) break;

        ecs_load__asset_entry_t *entry = &parsed_assets[*parsed_asset_count];
        asset_meta_t            *meta  = &entry->meta;
        u32 type_val = 0;
        sscanf(line_buf, "assetid:%u,assettype:%u,", &entry->asset_id, &type_val);
        meta->type = (asset_type)type_val;

        const char *path_part = strstr(line_buf, "assetpath:");
        if (!path_part) break;
        path_part += 10;

        if (meta->type == ASSET_TYPE_GLSL_SHADER)
        {
            char buf1[256] = {0}, buf2[256] = {0};
            sscanf(path_part, "[%255[^,],%255[^]]]", buf1, buf2);
            meta->filepath1 = str_init(arena, buf1);
            meta->filepath2 = str_init(arena, buf2);

            while (true)
            {
                memset(line_buf, 0, buf_size);
                file_readline(f, line_buf, buf_size);
                if (strncmp(line_buf, "\tuniform:", 9) != 0) break;
                memset(line_buf, 0, buf_size);
                file_readline(f, line_buf, buf_size);
            }
        }
        else if (meta->type == ASSET_TYPE_TEXTURE_SPRITE_ATLAS)
        {
            char buf[256] = {0};
            sscanf(path_part, "%255s", buf);
            meta->filepath1 = str_init(arena, buf);
            meta->meta.tile_counts = (vec2i_t){1, 1};

            while (true)
            {
                memset(line_buf, 0, buf_size);
                file_readline(f, line_buf, buf_size);
                if (strncmp(line_buf, "\ttilecount:[", 12) == 0)
                {
                    sscanf(line_buf, "\ttilecount:[%u,%u]", &meta->meta.tile_counts.x, &meta->meta.tile_counts.y);
                    continue;
                }
                break;
            }
        }
        else
        {
            char buf[256] = {0};
            sscanf(path_part, "%255s", buf);
            meta->filepath1 = str_init(arena, buf);

            memset(line_buf, 0, buf_size);
            file_readline(f, line_buf, buf_size);
        }
        (*parsed_asset_count)++;
    }
}

INTERNAL void ecs_serializer__internal_write_footer(file_t *const file)
{
    buffer(WORD) buffer = {0};
    snprintf(buffer.raw_data, sizeof(buffer.raw_data), "fin\n");
    file_writeline(file, (char *)buffer.raw_data);
}

