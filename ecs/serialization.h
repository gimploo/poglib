#pragma once
#include "poglib/ecs/common.h"
#include "poglib/ecs/component/types.h"
#include "poglib/util/assetmanager.h"

#define ECS_SAVE_FILE_MAGIC                             0x45435346u
#define ECS_SAVE_FILE_VERSION                           1.0
const str_t ECS_SAVE_FILEPATH                           = str_lit("ecs.save");
const str_t ECS_SAVE_FILE_HEADER_PREFIX                 = str_lit("ECS v");
const str_t ECS_DESERIALIZER_ENTITY_PREFIX              = str_lit("entity:");
const str_t ECS_DESERIALIZER_SIGNATURE_PREFIX           = str_lit("component_signature:");
const str_t ECS_DESERIALIZER_ASSETID_PREFIX             = str_lit("assetid:");
const str_t ECS_DESERIALIZER_FIN                        = str_lit("fin");

enum { ECS_LOAD_MAX_ENTITIES = 128, ECS_LOAD_MAX_ASSETS = 32 };

typedef struct {
    u32                   asset_id;
    asset_meta_t          meta;
    gluniform_registry_t  uniforms;
} ecs_load__asset_entry_t;

typedef struct {
    ecs_load__asset_entry_t data[ECS_LOAD_MAX_ASSETS];
    u32 count;
} ecs_load__asset_list_t;

typedef struct {
    ecs_componentbundle_t data[ECS_LOAD_MAX_ENTITIES];
    u32 count;
} ecs_load__entity_list_t;

const struct {
    ecs_component_type type;
    u8 idx;
    str_t prefix;
} ECS_DESERIALIZER__INTERNAL_CMP_PREFIX_MAP[ECS_CMP_COUNT] = {
    { ECS_CMP_TRANSFORM, ECS_CMP_TRANSFORM_IDX, str_lit("transform:") },
    { ECS_CMP_MODEL,     ECS_CMP_MODEL_IDX,     str_lit("model:")     },
    { ECS_CMP_INPUT,     ECS_CMP_INPUT_IDX,     str_lit("input:")     },
    { ECS_CMP_MATERIAL,  ECS_CMP_MATERIAL_IDX,  str_lit("material:")  },
    { ECS_CMP_CAMERA,    ECS_CMP_CAMERA_IDX,    str_lit("camera:")    },
    { ECS_CMP_COLLIDER,  ECS_CMP_COLLIDER_IDX,  str_lit("collider:")  },
    { ECS_CMP_MESH,      ECS_CMP_MESH_IDX,      str_lit("mesh:")      },
};

typedef enum {
    ECS_CMP_FLD_VEC3F,
    ECS_CMP_FLD_VERSORS,
    ECS_CMP_FLD_F32,
    ECS_CMP_FLD_VEC2F,
    ECS_CMP_FLD_I32,
    ECS_CMP_FLD_U32,
    ECS_CMP_FLD_U64,
    ECS_CMP_FLD_SKIP,
} ecs_cmp_fld_vtype_t;

typedef struct {
    str_t              prefix;
    const char         *scanf_fmt;
    u64                offset;
    ecs_cmp_fld_vtype_t vtype;
} ecs_cmp_field_descriptor_t;

typedef struct {
    u8 field_count;
    ecs_cmp_field_descriptor_t fields[9];
} ecs_cmp_field_map_t;

#define CMP_FLD_DESC(PREFIX, FMT, STRUCT, FIELD, FMT_TYPE) \
    { str_lit(PREFIX), FMT, offsetof(STRUCT, FIELD), FMT_TYPE }

static const ecs_cmp_field_map_t ecs_deserializer__internal_cmp_field_maps[ECS_CMP_COUNT] = {
    [ECS_CMP_TRANSFORM_IDX] = {
        .field_count = 5,
        .fields = {
            CMP_FLD_DESC("\tposition:",    "\tposition:[%f,%f,%f]",       ecs_component_transform_t, position,      ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\torientation:", "\torientation:[%f,%f,%f,%f]", ecs_component_transform_t, orientation,   ECS_CMP_FLD_VERSORS),
            CMP_FLD_DESC("\tscale:",       "\tscale:[%f,%f,%f]",          ecs_component_transform_t, scale,         ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\tvelocity:",    "\tvelocity:[%f,%f,%f]",       ecs_component_transform_t, velocity,      ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\tsource:",      "\tsource:%i",                 ecs_component_transform_t, source,        ECS_CMP_FLD_I32),
        }
    },
    [ECS_CMP_MODEL_IDX] = {
        .field_count = 1,
        .fields = {
            CMP_FLD_DESC("\tasset_id:", "\tasset_id:%lu", ecs_component_model_t, asset_id, ECS_CMP_FLD_U64),
        }
    },
    [ECS_CMP_INPUT_IDX] = {
        .field_count = 6,
        .fields = {
            CMP_FLD_DESC("\tdirection_source:",     "\tdirection_source:%i",                ecs_component_input_t,     direction_source,                    ECS_CMP_FLD_I32),
            CMP_FLD_DESC("\tinput_behavior:",       "\tinput_behavior:%p",                  ecs_component_input_t,     input_behavior,                      ECS_CMP_FLD_SKIP),
            CMP_FLD_DESC("\tstate.position:",       "\tstate.position:[%f,%f,%f]",          ecs_component_input_t,     internal.state.current_position,     ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\tstate.orientation:",    "\tstate.orientation:[%f,%f,%f,%f]",    ecs_component_input_t,     internal.state.current_orientation,  ECS_CMP_FLD_VERSORS),
            CMP_FLD_DESC("\tstate.front:",          "\tstate.front:[%f,%f,%f]",             ecs_component_input_t,     internal.state.front,                ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\tstate.right:",          "\tstate.right:[%f,%f,%f]",             ecs_component_input_t,     internal.state.right,                ECS_CMP_FLD_VEC3F),
        }
    },
    [ECS_CMP_MATERIAL_IDX] = {
        .field_count = 2,
        .fields = {
            CMP_FLD_DESC("\ttexture_asset_id:", "\ttexture_asset_id:%u", ecs_component_material_t, texture_asset_id, ECS_CMP_FLD_U32),
            CMP_FLD_DESC("\tshader_asset_id:",  "\tshader_asset_id:%u",  ecs_component_material_t, shader_asset_id,  ECS_CMP_FLD_U32),
        }
    },
    [ECS_CMP_CAMERA_IDX] = {
        .field_count = 9,
        .fields = {
            CMP_FLD_DESC("\tposition:",              "\tposition:[%f,%f,%f]",           ecs_component_camera_t, camera.position,            ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\teuler_angle:",           "\teuler_angle:[%f,%f]",           ecs_component_camera_t, camera.euler_angle,         ECS_CMP_FLD_VEC2F),
            CMP_FLD_DESC("\tdirection.front:",       "\tdirection.front:[%f,%f,%f]",    ecs_component_camera_t, camera.direction.front,     ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\tdirection.up:",          "\tdirection.up:[%f,%f,%f]",       ecs_component_camera_t, camera.direction.up,        ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\tdirection.right:",       "\tdirection.right:[%f,%f,%f]",    ecs_component_camera_t, camera.direction.right,     ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\tmode:",                  "\tmode:%i",                        ecs_component_camera_t, mode,                      ECS_CMP_FLD_I32),
            CMP_FLD_DESC("\tfollow.orbit_radius:",   "\tfollow.orbit_radius:%f",        ecs_component_camera_t, follow.orbit_radius,        ECS_CMP_FLD_F32),
            CMP_FLD_DESC("\tfollow.center_offset:",  "\tfollow.center_offset:[%f,%f,%f]",ecs_component_camera_t, follow.center_offset,      ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\tfollow.track_entity_id:","\tfollow.track_entity_id:%u",     ecs_component_camera_t, follow.track_entity_id,     ECS_CMP_FLD_U32),
        }
    },
    [ECS_CMP_COLLIDER_IDX] = {
        .field_count = 6,
        .fields = {
            CMP_FLD_DESC("\tshape_type:",        "\tshape_type:%i",           ecs_component_collider_t, shape_type,              ECS_CMP_FLD_I32),
            CMP_FLD_DESC("\tmotion_type:",       "\tmotion_type:%i",          ecs_component_collider_t, motion_type,             ECS_CMP_FLD_I32),
            CMP_FLD_DESC("\tobject_layer_type:", "\tobject_layer_type:%lu",   ecs_component_collider_t, object_layer_type,       ECS_CMP_FLD_U64),
            CMP_FLD_DESC("\tdim.cube:",          "\tdim.cube:[%f,%f,%f]",     ecs_component_collider_t, dim.cube.half_width,     ECS_CMP_FLD_VEC3F),
            CMP_FLD_DESC("\tdim.sphere:",        "\tdim.sphere:[%f]",         ecs_component_collider_t, dim.sphere.radius,       ECS_CMP_FLD_F32),
            CMP_FLD_DESC("\tdim.capsule:",       "\tdim.capsule:[%f,%f]",     ecs_component_collider_t, dim.capsule.radius,      ECS_CMP_FLD_VEC2F),
        }
    },
    [ECS_CMP_MESH_IDX] = {
        .field_count = 2,
        .fields = {
            CMP_FLD_DESC("\tasset_id:",              "\tasset_id:%lu",              ecs_component_mesh_t, asset_id,              ECS_CMP_FLD_U64),
            CMP_FLD_DESC("\tprototype_sprite_type:", "\tprototype_sprite_type:%i",  ecs_component_mesh_t, prototype_sprite_type, ECS_CMP_FLD_I32),
        }
    },
};


INTERNAL void ecs_serializer__internal_write_header(file_t *const file)
{
    const u32 version      = ECS_SAVE_FILE_VERSION;

    buffer(WORD) buffer = {0};
    snprintf((char *)buffer.raw_data, sizeof(buffer.raw_data), "ECS v%i save\n", version);
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

INTERNAL void ecs_serializer__internal_write_cmp_field(const ecs_cmp_field_descriptor_t *f, const u8 *src, file_t *const file)
{
    buffer(WORD) buf = {0};
    int len = 0;
    switch (f->vtype)
    {
        case ECS_CMP_FLD_VEC3F: {
            const vec3f_t *v    = (const vec3f_t *)src;
            len                 = snprintf((char *)buf.raw_data, sizeof(buf.raw_data), f->scanf_fmt, v->x, v->y, v->z);
        } break;
        case ECS_CMP_FLD_VERSORS: {
            const versors *q    = (const versors *)src;
            len                 = snprintf((char *)buf.raw_data, sizeof(buf.raw_data), f->scanf_fmt, q->x, q->y, q->z, q->w);
        } break;
        case ECS_CMP_FLD_F32:   len = snprintf((char *)buf.raw_data, sizeof(buf.raw_data), f->scanf_fmt, *(const f32 *)src);  break;
        case ECS_CMP_FLD_VEC2F: {
            const vec2f_t *v    = (const vec2f_t *)src;
            len                 = snprintf((char *)buf.raw_data, sizeof(buf.raw_data), f->scanf_fmt, v->x, v->y);
        } break;
        case ECS_CMP_FLD_I32:   len = snprintf((char *)buf.raw_data, sizeof(buf.raw_data), f->scanf_fmt, *(const i32 *)src);  break;
        case ECS_CMP_FLD_U32:   len = snprintf((char *)buf.raw_data, sizeof(buf.raw_data), f->scanf_fmt, *(const u32 *)src);  break;
        case ECS_CMP_FLD_U64:   len = snprintf((char *)buf.raw_data, sizeof(buf.raw_data), f->scanf_fmt, *(const u64 *)src);  break;
        default: return;
    }
    if (len > 0 && (u32)len < sizeof(buf.raw_data) - 1)
    {
        buf.raw_data[len] = '\n';
        file_writebytes(file, buf.raw_data, len + 1);
    }
}

INTERNAL void ecs_serializer__internal_entity_cmp_data(file_t *const file, const ecs_component_type type, const void *const cmp_data)
{
    for (u8 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        if (type != (1 << cmp_idx)) continue;

        u16 total_members   = 0;
        u16 bytesize        = 0;

        switch (type)
        {
            case ECS_CMP_TRANSFORM:     total_members = 5;   bytesize = sizeof(ecs_component_transform_t);   break;
            case ECS_CMP_MODEL:         total_members = 2;   bytesize = sizeof(ecs_component_model_t);       break;
            case ECS_CMP_INPUT:         total_members = 3;   bytesize = sizeof(ecs_component_input_t);       break;
            case ECS_CMP_MATERIAL:      total_members = 2;   bytesize = sizeof(ecs_component_material_t);    break;
            case ECS_CMP_CAMERA:        total_members = 3;   bytesize = sizeof(ecs_component_camera_t);      break;
            case ECS_CMP_COLLIDER:      total_members = 5;   bytesize = sizeof(ecs_component_collider_t);    break;
            case ECS_CMP_MESH:          total_members = 2;   bytesize = sizeof(ecs_component_mesh_t);        break;
            default: eprint("implementation missing");
        }

        buffer(WORD) header = {0};
        snprintf(
            (char *)header.raw_data, 
            sizeof(header.raw_data), 
            "%s{totalmembers:%u,bytesize:%u}\n",
            ECS_DESERIALIZER__INTERNAL_CMP_PREFIX_MAP[cmp_idx].prefix.data, 
            total_members, 
            bytesize
        );

        file_writebytes(file, header.raw_data, strlen((char *)header.raw_data));

        const ecs_cmp_field_map_t *map = &ecs_deserializer__internal_cmp_field_maps[cmp_idx];
        for (u8 i = 0; i < map->field_count; i++)
        {
            const ecs_cmp_field_descriptor_t *f = &map->fields[i];
            if (f->vtype == ECS_CMP_FLD_SKIP) continue;
            ecs_serializer__internal_write_cmp_field(f, (u8 *)cmp_data + f->offset, file);
        }
        return;
    }
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

INTERNAL void ecs_serializer__internal_write_footer(file_t *const file)
{
    buffer(WORD) buffer = {0};
    snprintf(buffer.raw_data, sizeof(buffer.raw_data), "fin\n");
    file_writeline(file, (char *)buffer.raw_data);
}

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

            if (f->vtype == ECS_CMP_FLD_SKIP) return;

            u8 *dest = (u8 *)cmp_data + f->offset;
            switch (f->vtype)
            {
                case ECS_CMP_FLD_VEC3F: {
                    vec3f_t *v = (vec3f_t *)dest;
                    sscanf(line, f->scanf_fmt, &v->x, &v->y, &v->z);
                } break;
                case ECS_CMP_FLD_VERSORS: {
                    versors *q = (versors *)dest;
                    sscanf(line, f->scanf_fmt, &q->x, &q->y, &q->z, &q->w);
                } break;
                case ECS_CMP_FLD_F32:  sscanf(line, f->scanf_fmt, (f32 *)dest);           break;
                case ECS_CMP_FLD_VEC2F: {
                    vec2f_t *v = (vec2f_t *)dest;
                    sscanf(line, f->scanf_fmt, &v->x, &v->y);
                } break;

                case ECS_CMP_FLD_I32:       sscanf(line, f->scanf_fmt, (i32 *)dest);     break;
                case ECS_CMP_FLD_U32:       sscanf(line, f->scanf_fmt, (u32 *)dest);     break;
                case ECS_CMP_FLD_U64:       sscanf(line, f->scanf_fmt, (u64 *)dest);     break;

                default: break;
            }
            return;
        }
    }
}

INTERNAL u32 ecs_deserializer__internal_ensure_asset_loaded(assetmanager_t *const assets, const u32 asset_id, const ecs_load__asset_list_t *const assets_parsed)
{
    const ecs_load__asset_entry_t *parsed = NULL;
    for (u32 i = 0; i < assets_parsed->count; i++)
    {
        if (assets_parsed->data[i].asset_id == asset_id) { parsed = &assets_parsed->data[i]; break; }
    }
    if (!parsed) return asset_id;

    hashtable_iterator(&assets->assetmeta_lookup, iter)
    {
        const hashtable_entry_t *entry     = iter;
        const asset_meta_t      *existing  = entry->value;
        if (existing->type != parsed->meta.type) continue;
        if (existing->filepath1.len && !str_cmp(existing->filepath1, parsed->meta.filepath1)) continue;
        if (existing->filepath2.len && !str_cmp(existing->filepath2, parsed->meta.filepath2)) continue;
        return entry->key.u32;
    }

    u32 new_id = 0;
    switch (parsed->meta.type)
    {
        case ASSET_TYPE_MODEL:
            new_id = assetmanager_load_model_async(assets, parsed->meta.filepath1);
            break;
        case ASSET_TYPE_GLSL_SHADER:
            new_id = assetmanager_load_glsl_shader(assets, parsed->meta.filepath1, parsed->meta.filepath2, parsed->uniforms);
            break;
        case ASSET_TYPE_TEXTURE_SPRITE_ATLAS:
            new_id = assetmanager_load_spriteatlas(assets, parsed->meta.filepath1, parsed->meta.meta.tile_counts.x, parsed->meta.meta.tile_counts.y);
            break;
        default: break;
    }
    return new_id ? new_id : asset_id;
}

INTERNAL void ecs_deserializer__internal_remap_entity_assets(ecs_componentbundle_t *const bundle, assetmanager_t *const assets, const ecs_load__asset_list_t *const assets_parsed)
{
    for (u8 cmp_idx = 0; cmp_idx < ECS_CMP_COUNT; cmp_idx++)
    {
        const ecs_component_type cmp_type = 1 << cmp_idx;
        if ((bundle->signature & cmp_type) == 0) continue;

        switch (cmp_type)
        {
            case ECS_CMP_MODEL: {
                ecs_component_model_t *m = &bundle->component[cmp_idx].model;
                m->asset_id = ecs_deserializer__internal_ensure_asset_loaded(assets, m->asset_id, assets_parsed);
            } break;
            case ECS_CMP_MESH: {
                ecs_component_mesh_t *mesh = &bundle->component[cmp_idx].mesh;
                //NOTE: assets with such ids are coming from the workbench's primitve types that already loaded by the workbench, although
                //when shipping the game workbench needs to be disabled along with this primitves replaced with proper optimized meshes
                if (mesh->asset_id < GL_MESH_PRIMITIVE_TYPE_COUNT) {
#ifndef DEBUG
                    eprint("workbench meshes are not to be used in the release build of the game - replace those meshes with proper optimized ones");
#endif
                    continue;
                }
                mesh->asset_id = ecs_deserializer__internal_ensure_asset_loaded(assets, mesh->asset_id, assets_parsed);
            } break;
            case ECS_CMP_MATERIAL: {
                ecs_component_material_t *mat = &bundle->component[cmp_idx].material;
                if (mat->texture_asset_id)  mat->texture_asset_id = ecs_deserializer__internal_ensure_asset_loaded(assets, mat->texture_asset_id, assets_parsed);
                if (mat->shader_asset_id)   mat->shader_asset_id  = ecs_deserializer__internal_ensure_asset_loaded(assets, mat->shader_asset_id,  assets_parsed);
            } break;
        }
    }
}

INTERNAL void ecs_deserializer__internal__read_assetmeta_section(file_t *const f, ecs_load__asset_list_t *const assets_parsed, arena_t *arena, char *line_buf, u32 buf_size)
{
    while (strncmp(line_buf, ECS_DESERIALIZER_FIN.data, ECS_DESERIALIZER_FIN.len) != 0)
    {
        if (strncmp(line_buf, ECS_DESERIALIZER_ASSETID_PREFIX.data, ECS_DESERIALIZER_ASSETID_PREFIX.len) != 0)
        {
            memset(line_buf, 0, buf_size);
            file_readline(f, line_buf, buf_size);
            continue;
        }

        if (assets_parsed->count >= ECS_LOAD_MAX_ASSETS) break;

        ecs_load__asset_entry_t *entry = &assets_parsed->data[assets_parsed->count];
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
                char name[128] = {0};
                u32  utype     = 0;
                if (sscanf(line_buf, "\t\tname:%127[^,],type:%u", name, &utype) == 2
                    && entry->uniforms.count < MAX_UNIFORMS_ALLOWED_IN_SHADER)
                {
                    entry->uniforms.data[entry->uniforms.count++] = (gluniform_meta_t){
                        .name = str_init(arena, name),
                        .type = (gluniform_type)utype,
                    };
                }
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
        else if (meta->type == ASSET_TYPE_MODEL)
        {
            char buf[256] = {0};
            sscanf(path_part, "%255s", buf);
            meta->filepath1 = str_init(arena, buf);

            memset(line_buf, 0, buf_size);
            file_readline(f, line_buf, buf_size);

        } else {

            eprint("implementation missing");
        }
        assets_parsed->count++;
    }
}


