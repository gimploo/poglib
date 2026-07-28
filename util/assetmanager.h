#pragma once
#include <poglib/basic.h>
#include "./asset.h"
#include <poglib/sound.h>
#include "poglib/basic/concurrency.h"
#include "poglib/gfx/gl/objects.h"
#include "poglib/gfx/gl/renderconfig.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/gfx/gl/types.h"
#include "poglib/gfx/gl/vbo_stream_types.h"
#include "poglib/gfx/model/assimp.h"
#include "poglib/util/spriteatlas.h"
#include "poglib/util/workbench/workbench-constants.h"

/*=====================================================================================================
                                -- ASSET MANAGER --
=====================================================================================================*/


typedef struct assetmanager_t assetmanager_t;
struct assetmanager_t {
    arena_t            *arena;
    bgtask_manager_t    *bgtask_manager;
    hashtable_t         assetmaps[ASSET_TYPE_COUNT];
    hashtable_t         gpu_uploaded_assets;
    hashtable_t         assetmeta_lookup;
    struct {
        u32             asset_idx_generator;
        mpsc_queue_t    asset_staging_queue;
        queue_t         notification_queue;
    } internal;
};


assetmanager_t      assetmanager_init(bgtask_manager_t *const taskmanager);
void                assetmanager_update(assetmanager_t *const self);

void                assetmanager_load_all_primitives(assetmanager_t * const self);
u32                 assetmanager_load_model_async(assetmanager_t *self, const str_t filepath);
u32                 assetmanager_load_spriteatlas(assetmanager_t *const self, const str_t filepath, const u32 tile_count_width, const u32 tile_count_height);
u32                 assetmanager_load_texture(assetmanager_t *self, const str_t filepath);
u32                 assetmanager_load_glsl_shader(assetmanager_t *self, const str_t vtx_filepath, const str_t frag_filepath, const gluniform_registry_t registry);
wwise_audio_t       assetmanager_load_audios_from_wwise(assetmanager_t *const self);
u32                 assetmanager_load_audio_music_async(assetmanager_t *const self, const audio_musiclayer_t musiclayers[AUDIO_MUSIC_LAYERS_MAX_COUNT], const u8 layer_count);
u32                 assetmanager_load_audio_sfx_async(assetmanager_t *const self, const str_t filepath);
wwise_audio_t       assetmanager_load_audios_from_wwise(assetmanager_t *const self);

queue_t *           assetmanager_get_notificationqueue(assetmanager_t *const self);
const void *        assetmanager_get_assetresource(const assetmanager_t *const self, const asset_type assettype, const u32 assetId);
gpu_asset_t *       assetmanager_get_gpu_loaded_asset_async(const assetmanager_t *const self, const u32 asset_id);

void                assetmanager_destroy(assetmanager_t *const self);



#ifndef IGNORE_ASSETMANAGER_IMPLEMENTATION

typedef struct {
    enum {
        ASSET_EVENT_MODEL_LOADED_TO_GPU = 1,
        ASSET_EVENT_SHADER_COMPILED     = 2,
        ASSET_EVENT_AUDIO_THEME_LOADED  = 3,
        ASSET_EVENT_AUDIO_SFX_LOADED    = 4
    } type;
    struct {
        u32             id;
        asset_meta_t    meta;
    } asset;
} asset_loaded_event_t;

#define MAX_ASSETS_ALLOWED_PER_TYPE 10

INTERNAL void assetmanager__internal_add_asset_meta_data(assetmanager_t *const self, const asset_type type, const u32 asset_id, const str_t filepath1, const str_t filepath2, void *const asset_data);
INTERNAL void assetmanager__internal_write_uniformlocs_to_file(const hashtable_entry_t *const entry, buffer_t *const buffer);
INTERNAL void assetmanager__internal__notify(const taskparams_t params);

assetmanager_t assetmanager_init(bgtask_manager_t *const taskmanager)
{
    ASSERT(taskmanager);
    arena_t *arena = arena_init(NULL, 1 * MB);
    assetmanager_t result = {
        .arena = arena,
        .assetmaps = {
            [ASSET_TYPE_MODEL]                  = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(async(glmodel_t)),  .type = HT_STORAGE_BY_REFERENCE }, arena),
            [ASSET_TYPE_GLSL_SHADER]            = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(glshader_t),        .type = HT_STORAGE_BY_REFERENCE }, arena),
            [ASSET_TYPE_TEXTURE]                = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(gltexture2d_t),     .type = HT_STORAGE_BY_REFERENCE }, arena),
            [ASSET_TYPE_TEXTURE_SPRITE_ATLAS]   = hashtable_init(3, HT_KEY_TYPE_U32,    (ht_value_type) { .size = sizeof(spriteatlas_t),        .type = HT_STORAGE_BY_VALUE     }, arena),
            [ASSET_TYPE_MUSIC_SFX]              = hashtable_init(10, HT_KEY_TYPE_U32,   (ht_value_type) { .size = sizeof(async(audio_sfx_t)),   .type = HT_STORAGE_BY_REFERENCE }, arena),
            [ASSET_TYPE_MUSIC_THEME]            = hashtable_init(5, HT_KEY_TYPE_U32,    (ht_value_type) { .size = sizeof(async(audio_music_t)), .type = HT_STORAGE_BY_REFERENCE }, arena),
        },
        .bgtask_manager         = taskmanager,
        .assetmeta_lookup       = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE * ASSET_TYPE_COUNT, HT_KEY_TYPE_U32, (ht_value_type){ .size = sizeof(asset_meta_t), .type = HT_STORAGE_BY_VALUE } , arena),
        .gpu_uploaded_assets    = hashtable_init(ASSET_TYPE_COUNT * MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(gpu_asset_t), .type = HT_STORAGE_BY_REFERENCE }, arena),
        .internal = {
            .asset_idx_generator        = GL_MESH_PRIMITIVE_TYPE_COUNT,
            .asset_staging_queue        = mpsc_queue(arena, MAX_ASSETS_ALLOWED_PER_TYPE * ASSET_TYPE_COUNT),
            .notification_queue         = queue_init(10, asset_loaded_event_t, arena)
        }
    };
    return result;
}

void assetmanager__internal_assetmaps_destroy(assetmanager_t *const self)
{
    for (asset_type type = 0; type < ASSET_TYPE_COUNT; type++)
    {
        hashtable_iterator(&self->assetmaps[type], iter)
        {
            hashtable_entry_t *const entry = iter;
            switch(type)
            {
                case ASSET_TYPE_MUSIC_SFX:
                    audio_sfx_destroy(((taskresponse_t *)entry->value)->resource); 
                break;
                case ASSET_TYPE_MUSIC_THEME:
                    audio_music_destroy(((taskresponse_t *)entry->value)->resource); 
                break;
                case ASSET_TYPE_MODEL: 
                    glmodel_destroy(((taskresponse_t *)entry->value)->resource); 
                break;
                case ASSET_TYPE_GLSL_SHADER: 
                    glshader_destroy(entry->value);
                break;
                case ASSET_TYPE_TEXTURE:
                    gltexture2d_destroy(entry->value);
                break;
                case ASSET_TYPE_TEXTURE_SPRITE_ATLAS:
                    spriteatlas_destroy(entry->value);
                break;
                default: eprint("asset type not implemented");
            }
        }
    }
}

void assetmanager__internal_thread_callback_load_glmodel(const taskparams_t params, taskstorage_t storage, taskresponse_t *const response) 
{
    ASSERT(response->resource);
    ASSERT(params.count == 3);
    ASSERT(storage.buffer.raw_data);
    ASSERT(storage.buffer.size >= sizeof(asset_staging_event_t));

    const str_t filepath = params.arg[0].str;
    *(glmodel_t *)response->resource = glmodel_init(filepath);

    memcpy(
        storage.buffer.raw_data, 
        &(asset_staging_event_t){
            .asset_id       = params.arg[2].u64,
            .type           = ASSET_TYPE_MODEL,
            .processed_data = response->resource,
        }, 
        sizeof(asset_staging_event_t)
    );

    mpsc_queue_t * const queue = (mpsc_queue_t *)params.arg[1].any;
    mpsc_queue_put(queue, storage.buffer.raw_data);
}

u32 assetmanager_load_model_async(assetmanager_t *const self, const str_t filepath)
{
    const u32 asset_id = ++self->internal.asset_idx_generator;

    taskresponse_t *response = taskresponse(self->arena, sizeof(glmodel_t));
    hashtable_insert(
        &self->assetmaps[ASSET_TYPE_MODEL], 
        (hashtable_key_t){ .u32 = asset_id }, 
        response
    );
    assetmanager__internal_add_asset_meta_data(self, ASSET_TYPE_MODEL, asset_id, filepath, STR_EMPTY, NULL);

    bgtask_manager_pass_task(
        self->bgtask_manager,
        (taskrequest_t){
            .response = response,
            .params = {
                    .count = 3,
                    .arg = {
                        [0].str = filepath,
                        [1].any = &self->internal.asset_staging_queue,
                        [2].u64 = asset_id
                    }
            },
            .storage = {
                .buffer = buffer_init(NULL, sizeof(asset_staging_event_t))
            },
            .callback = assetmanager__internal_thread_callback_load_glmodel,
        }
    );

    return asset_id;
}

const void * assetmanager_get_assetresource(const assetmanager_t *const self, const asset_type assettype, const u32 assetId)
{
    ASSERT(assettype >= 0 && assettype < ASSET_TYPE_COUNT);
    ASSERT(assetId != INVALID_ASSET_ID);

    if (!hashtable_has_key(&self->assetmaps[assettype], (hashtable_key_t){ .u32 = assetId })) {
        eprint("asset id `%i` is not a valid identifier for asset type `%i`", assetId, assettype);
    }

    const void *entry = hashtable_get_value(
        &self->assetmaps[assettype], 
        (hashtable_key_t){ .u32 = assetId }
    );

    const taskresponse_t *const response = (taskresponse_t *)entry;
    return ASSET_ASYNC_LOADING_SUPPORT[assettype]
        ? (response->is_done ? response->resource : NULL)
        : entry;
}

void assetmanager_destroy(assetmanager_t *const self) 
{
    ASSERT(self);

    while(glGetError() != GL_NO_ERROR) {}

    hashtable_iterator(&self->gpu_uploaded_assets, entry)
    {
        gpu_asset_t *const asset = ((hashtable_entry_t * )entry)->value;
        for (u16 idx = 0; idx < asset->meshes.count; idx++) {
            const gpu_mesh_t gpu_mesh = asset->meshes.data[idx];
            vao_destroy(&(vao_t){.id  = gpu_mesh.vao_id});
        }

    }
    assetmanager__internal_assetmaps_destroy(self);
    arena_destroy(self->arena);
}

u32 assetmanager_load_glsl_shader(assetmanager_t *const self, const str_t vtx_filepath, const str_t frag_filepath, const gluniform_registry_t registry) 
{
    hashtable_iterator(&self->assetmaps[ASSET_TYPE_GLSL_SHADER], iter)
    {
        const hashtable_entry_t *const table_entry = iter;
        glshader_t *const shader = table_entry->value;

        if (str_cmp(shader->fg, frag_filepath) && str_cmp(shader->vs, vtx_filepath) && registry.count == shader->internal.uniformlocs.entries.len) 
            return table_entry->key.u32;
    }

    const u32 asset_id = ++self->internal.asset_idx_generator;

    glshader_t *shader = arena_reserve(self->arena, sizeof(glshader_t));
    *shader = glshader_init(vtx_filepath, frag_filepath, registry, self->arena);

    assetmanager__internal_add_asset_meta_data(self, ASSET_TYPE_GLSL_SHADER, asset_id, vtx_filepath, frag_filepath, shader);
    hashtable_insert(&self->assetmaps[ASSET_TYPE_GLSL_SHADER], (hashtable_key_t){.u32 = asset_id}, shader);
    logging("Shader compiled %s, %s", shader->fg.data, shader->vs.data);

    return asset_id;
}

void assetmanager__internal__upload_model_to_gpu(
    assetmanager_t *const self,
    const glmodel_t *const model,
    const u32 asset_id
) {
    gpu_asset_t * const gpu_asset           = arena_reserve(self->arena, sizeof(gpu_asset_t));
    gpu_asset->asset_id                     = asset_id;
    gpu_asset->meshes.count                 = model->meshes.len;
    gpu_asset->meshes.data                  = arena_reserve(self->arena, sizeof(gpu_mesh_t) * gpu_asset->meshes.count);

    list_iterator(&model->meshes, iter) 
    {
        const glmesh_t *const cpu_mesh = iter;

        vao_t vao = vao_init();
        vao_bind(&vao);

        vbo_t vbo = vbo_init((vbo_config_t) {
            .usage = GL_STATIC_DRAW,
            .chunks = {
                [VBO_STREAM_TYPE_GEOMETRY] = { 
                    .buffer = {
                        .raw_data = cpu_mesh->vtx.data,
                        .size = slot_get_size(&cpu_mesh->vtx),
                    }, 
                    .index_count = cpu_mesh->idx.len
                },
                [VBO_STREAM_TYPE_INSTANCE] = {0},
            }
        });

        ebo_init(&vbo, (u32 *)cpu_mesh->idx.data, cpu_mesh->idx.len);

        const glvtx_attributelist_t attrlist = glmodel_get_attirbutelist();
        for(u16 attr_idx = 0; attr_idx < attrlist.count; attr_idx++)
        {
            vao_set_attributes(
                &vao,
                &vbo, 
                attrlist.attr[attr_idx].ncmp, 
                attrlist.attr[attr_idx].type, 
                false, 
                attrlist.attr[attr_idx].interleaved.stride, 
                attrlist.attr[attr_idx].interleaved.offset, 
                false,
                VBO_STREAM_TYPE_GEOMETRY
            );
        }

        gpu_asset->meshes.data[(u64)list_iterator_index] = (gpu_mesh_t){
            .vao_id = vao.id,
            .index_count = cpu_mesh->idx.len,
            .attribute_count = vbo.internals.attribute_index + 1
        };

        vao_unbind();
    }

    list_iterator(&model->textures, iter) {
        model_texture_t *mt = (model_texture_t *)iter;
        gltexture2d_upload_to_gpu(&mt->texture);
    }

    logging("Model "STR_FMT" loaded to GPU", STR_ARG(model->filepath));

    hashtable_insert(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id }, gpu_asset);
}

INTERNAL void assetmanager__internal__process_loaded_assets(assetmanager_t *const self)
{
    mpsc_queue_t *const queue = &self->internal.asset_staging_queue;

    while(true) {
        asset_staging_event_t *const task = (asset_staging_event_t *)mpsc_queue_get(queue);
        if (!task) {
            return;
        }

        if (!ASSET_ASYNC_LOADING_SUPPORT[task->type]) continue;

        switch(task->type)
        {
            case ASSET_TYPE_MODEL:
                assetmanager__internal__upload_model_to_gpu(self, (glmodel_t *)task->processed_data, task->asset_id);
                queue_put(
                    &self->internal.notification_queue, 
                    &(asset_loaded_event_t) { 
                        .type = ASSET_EVENT_MODEL_LOADED_TO_GPU,
                        .asset = {
                            .id     = task->asset_id,
                            .meta   = *(asset_meta_t *)hashtable_get_value(&self->assetmeta_lookup, (hashtable_key_t){ .u32 = task->asset_id }),
                        }
                    }, 
                    sizeof(asset_loaded_event_t)
                );
            break;
            case ASSET_TYPE_MUSIC_SFX:
                queue_put(
                    &self->internal.notification_queue, 
                    &(asset_loaded_event_t) { 
                        .type = ASSET_EVENT_AUDIO_SFX_LOADED,
                        .asset = { .id = task->asset_id }
                    }, 
                    sizeof(asset_loaded_event_t)
                );
            break;
            case ASSET_TYPE_MUSIC_THEME:
                queue_put(
                    &self->internal.notification_queue, 
                    &(asset_loaded_event_t) { 
                        .type = ASSET_EVENT_AUDIO_THEME_LOADED,
                        .asset = { .id = task->asset_id }
                    }, 
                    sizeof(asset_loaded_event_t)
                );
            break;
        }

        mem_free(task, sizeof(*task));

    }
}

void assetmanager_update(assetmanager_t *const self)
{
    ASSERT(self);
    queue_clear(&self->internal.notification_queue);
    assetmanager__internal__process_loaded_assets(self);
}

void assetmanager__internal__upload_cube_to_gpu(assetmanager_t *const self)
{
    const u32 asset_id = GL_MESH_PRIMITIVE_TYPE_CUBE;

    vao_t vao = vao_init();
    vao_bind(&vao);

    vbo_t vbo = vbo_init((vbo_config_t) {
        .usage = GL_STATIC_DRAW,
        .chunks = {
            [VBO_STREAM_TYPE_GEOMETRY] = { 
                .buffer = {
                    .raw_data = (u8 *)DEFAULT_CUBE_VERTICES_WITH_NORMALS_AND_UVS_24,
                    .size = sizeof(DEFAULT_CUBE_VERTICES_WITH_NORMALS_AND_UVS_24),
                }, 
                .index_count = ARRAY_LEN(DEFAULT_CUBE_INDICES_24)
            },
            [VBO_STREAM_TYPE_INSTANCE] = {0},
        }
    });

    const ebo_t ebo = ebo_init(&vbo, DEFAULT_CUBE_INDICES_24, ARRAY_LEN(DEFAULT_CUBE_INDICES_24));

    //Pos
    vao_set_attributes(
        &vao,
        &vbo, 
        3, 
        GL_FLOAT, 
        false, 
        sizeof(f32) * 8, 
        0, 
        false,
        VBO_STREAM_TYPE_GEOMETRY
    );

    //Norm
    vao_set_attributes(
        &vao,
        &vbo, 
        3, 
        GL_FLOAT, 
        false, 
        sizeof(f32) * 8, 
        sizeof(f32) * 3, 
        false,
        VBO_STREAM_TYPE_GEOMETRY
    );

    //UV
    vao_set_attributes(
        &vao,
        &vbo, 
        2, 
        GL_FLOAT, 
        false, 
        sizeof(f32) * 8, 
        sizeof(f32) * 6, 
        false,
        VBO_STREAM_TYPE_GEOMETRY
    );
    vao_unbind();


    gpu_mesh_t * const gpu_mesh = arena_store(
        self->arena, 
        &(gpu_mesh_t) {
            .vao_id = vao.id,
            .index_count = ebo.indices_count,
            .attribute_count = vbo.internals.attribute_index + 1
        },
        sizeof(gpu_mesh_t)
    );

    gpu_asset_t * const gpu_asset = arena_store(
        self->arena, 
        &(gpu_asset_t) {
            .asset_id = asset_id,
            .meshes = {
            .count = 1,
                .data = gpu_mesh
            }
        },
        sizeof(gpu_asset_t));

    hashtable_insert(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id }, gpu_asset);
}


void assetmanager__internal__upload_capsule_to_gpu(assetmanager_t *const self)
{
    const u32 asset_id = GL_MESH_PRIMITIVE_TYPE_CAPSULE;

    vao_t vao = vao_init();
    vao_bind(&vao);

    vbo_t vbo = vbo_init((vbo_config_t) {
        .usage = GL_STATIC_DRAW,
        .chunks = {
            [VBO_STREAM_TYPE_GEOMETRY] = { 
                .buffer = {
                    .raw_data = (u8 *)DEFAULT_CAPSULE_VERTICES_WITH_NORMALS,
                    .size = sizeof(DEFAULT_CAPSULE_VERTICES_WITH_NORMALS),
                }, 
                .index_count = ARRAY_LEN(DEFAULT_CAPSULE_INDICES)
            },
            [VBO_STREAM_TYPE_INSTANCE] = {0},
        }
    });

    const ebo_t ebo = ebo_init(&vbo, DEFAULT_CAPSULE_INDICES, ARRAY_LEN(DEFAULT_CAPSULE_INDICES));

    //Pos
    vao_set_attributes(
        &vao,
        &vbo, 
        3, 
        GL_FLOAT, 
        false, 
        sizeof(f32) * 6, 
        0, 
        false,
        VBO_STREAM_TYPE_GEOMETRY
    );

    //Norm
    vao_set_attributes(
        &vao,
        &vbo, 
        3, 
        GL_FLOAT, 
        false, 
        sizeof(f32) * 6, 
        sizeof(f32) * 3, 
        false,
        VBO_STREAM_TYPE_GEOMETRY
    );

    vao_unbind();

    gpu_mesh_t * const gpu_mesh = arena_store(
        self->arena, 
        &(gpu_mesh_t) {
            .vao_id = vao.id,
            .index_count = ebo.indices_count,
            .attribute_count = vbo.internals.attribute_index + 1
        },
        sizeof(gpu_mesh_t));

    const gpu_asset_t * const gpu_asset = arena_store(
        self->arena, 
        &(gpu_asset_t) {
            .asset_id = asset_id,
            .meshes = {
            .count = 1,
                .data = gpu_mesh
            }
        },
        sizeof(gpu_asset_t));

    hashtable_insert(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id }, gpu_asset);
}


void assetmanager__internal__upload_cylinder_to_gpu(assetmanager_t *const self)
{
    const u32 asset_id = GL_MESH_PRIMITIVE_TYPE_CYLINDER;

    vao_t vao = vao_init();
    vao_bind(&vao);

    vbo_t vbo = vbo_init((vbo_config_t) {
        .usage = GL_STATIC_DRAW,
        .chunks = {
            [VBO_STREAM_TYPE_GEOMETRY] = { 
                .buffer = {
                    .raw_data = (u8 *)DEFAULT_CYLINDER_VERTICES_WITH_NORMALS_AND_UVS,
                    .size = sizeof(DEFAULT_CYLINDER_VERTICES_WITH_NORMALS_AND_UVS),
                }, 
                .index_count = ARRAY_LEN(DEFAULT_CYLINDER_INDICES)
            },
            [VBO_STREAM_TYPE_INSTANCE] = {0},
        }
    });

    const ebo_t ebo = ebo_init(&vbo, DEFAULT_CYLINDER_INDICES, ARRAY_LEN(DEFAULT_CYLINDER_INDICES));

    vao_set_attributes(
        &vao,
        &vbo, 
        3, 
        GL_FLOAT, 
        false, 
        sizeof(f32) * 8, 
        0, 
        false,
        VBO_STREAM_TYPE_GEOMETRY
    );

    vao_set_attributes(
        &vao,
        &vbo, 
        3, 
        GL_FLOAT, 
        false, 
        sizeof(f32) * 8, 
        sizeof(f32) * 3, 
        false,
        VBO_STREAM_TYPE_GEOMETRY
    );

    vao_set_attributes(
        &vao,
        &vbo, 
        2, 
        GL_FLOAT, 
        false, 
        sizeof(f32) * 8, 
        sizeof(f32) * 6, 
        false,
        VBO_STREAM_TYPE_GEOMETRY
    );
    vao_unbind();

    gpu_mesh_t * const gpu_mesh = arena_store(
        self->arena, 
        &(gpu_mesh_t) {
            .vao_id = vao.id,
            .index_count = ebo.indices_count,
            .attribute_count = vbo.internals.attribute_index + 1
        },
        sizeof(gpu_mesh_t)
    );

    gpu_asset_t * const gpu_asset = arena_store(
        self->arena, 
        &(gpu_asset_t) {
            .asset_id = asset_id,
            .meshes = {
            .count = 1,
                .data = gpu_mesh
            }
        },
        sizeof(gpu_asset_t));

    hashtable_insert(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id }, gpu_asset);
}


void assetmanager__internal__upload_camera_model(assetmanager_t *const self)
{
    const u32 asset_id = GL_MESH_PRIMITIVE_TYPE_CAMERA;

    vao_t vao = vao_init();
    vao_bind(&vao);

    vbo_t vbo = vbo_init((vbo_config_t) {
        .usage = GL_STATIC_DRAW,
        .chunks = {
            [VBO_STREAM_TYPE_GEOMETRY] = { 
                .buffer = {
                    .raw_data = (u8 *)CAMERA_VERTICES,
                    .size = sizeof(CAMERA_VERTICES),
                }, 
                .index_count = ARRAY_LEN(CAMERA_INDICES)
            },
            [VBO_STREAM_TYPE_INSTANCE] = {0},
        }
    });

    const ebo_t ebo = ebo_init(&vbo, CAMERA_INDICES, ARRAY_LEN(CAMERA_INDICES));

    //Pos
    vao_set_attributes(
        &vao,
        &vbo, 
        3, 
        GL_FLOAT, 
        false, 
        sizeof(f32) * 3, 
        0, 
        false,
        VBO_STREAM_TYPE_GEOMETRY
    );

    vao_unbind();


    gpu_mesh_t *const gpu_mesh = arena_store(
        self->arena, 
        &(gpu_mesh_t) {
            .vao_id = vao.id,
            .index_count = ebo.indices_count,
            .attribute_count = vbo.internals.attribute_index + 1
        },
        sizeof(gpu_mesh_t));

    const gpu_asset_t *const gpu_asset = arena_store(
        self->arena, 
        &(gpu_asset_t) {
            .asset_id = asset_id,
            .meshes = {
            .count = 1,
                .data = gpu_mesh
            }
        },
        sizeof(gpu_asset_t));

    hashtable_insert(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id }, gpu_asset);
}

void assetmanager_load_all_primitives(assetmanager_t *const self)
{
    assetmanager__internal__upload_cube_to_gpu(self);
    assetmanager__internal__upload_capsule_to_gpu(self);
    assetmanager__internal__upload_cylinder_to_gpu(self);
    assetmanager__internal__upload_camera_model(self);

    logging("loaded all primitive shapes");
}

gpu_asset_t * assetmanager_get_gpu_loaded_asset_async(const assetmanager_t *const self, const u32 asset_id)
{
    return (gpu_asset_t *)hashtable_get_value_or_null(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id });
}

u32 assetmanager_load_spriteatlas(assetmanager_t *const self, const str_t filepath, const u32 tile_count_width, const u32 tile_count_height)
{
    hashtable_iterator(&self->assetmaps[ASSET_TYPE_TEXTURE_SPRITE_ATLAS], iter)
    {
        const hashtable_entry_t *const table_entry = iter;
        spriteatlas_t *const atlas = table_entry->value;

        if (str_cmp(atlas->texture.filepath, filepath)) 
            return table_entry->key.u32;
    }

    const spriteatlas_t atlas   = spriteatlas_init(filepath, tile_count_width, tile_count_height, self->arena);
    const u32 asset_id          = ++self->internal.asset_idx_generator;
    hashtable_insert(&self->assetmaps[ASSET_TYPE_TEXTURE_SPRITE_ATLAS], (hashtable_key_t){ .u32 = asset_id }, &atlas);
    assetmanager__internal_add_asset_meta_data(self,ASSET_TYPE_TEXTURE_SPRITE_ATLAS, asset_id, filepath, STR_EMPTY, (void *)&atlas);
    return asset_id;
}

INTERNAL void assetmanager__internal_add_asset_meta_data(
    assetmanager_t *const self, 
    const asset_type asset_type, 
    const u32 asset_id, 
    const str_t filepath1, 
    const str_t filepath2,
    void *const asset_data
) {
    ASSERT(filepath1.len || filepath2.len);

    asset_meta_t assetmeta = {
        .type = asset_type,
        .filepath1 = filepath1, 
        .filepath2 = filepath2, 
    };

    switch(asset_type)
    {
        case ASSET_TYPE_GLSL_SHADER:            assetmeta.meta.uniformlocs = &((glshader_t *)asset_data)->internal.uniformlocs;     break;
        case ASSET_TYPE_TEXTURE_SPRITE_ATLAS:   assetmeta.meta.tile_counts = ((spriteatlas_t *)asset_data)->tile_count;             break;
        default: break;
    }
 
    hashtable_insert_by_value(
        &self->assetmeta_lookup, 
        (hashtable_key_t) { .u32 = asset_id }, 
        &assetmeta
    );
}

u32 assetmanager_load_texture(assetmanager_t *self, const str_t filepath)
{
    eprint("not implemented");
}


queue_t * assetmanager_get_notificationqueue(assetmanager_t *const self)
{
    return &self->internal.notification_queue;
}


INTERNAL void assetmanager__internal__thread_callback_load_audio(const taskparams_t params, taskstorage_t storage, taskresponse_t *const response) 
{
    const asset_type type   = params.arg[0].u64;
    const u32 asset_id      = params.arg[1].u64;

    if (type == ASSET_TYPE_MUSIC_SFX)
    {
        ASSERT(params.count == 4);
        const str_t filepath    = params.arg[2].str;
        arena_t *arena          = params.arg[3].arena;
        *(audio_sfx_t *)response->resource = audio_sfx_init(&global_audio_engine, arena, filepath);
    }

    if (type == ASSET_TYPE_MUSIC_THEME)
    {
        ASSERT(params.count == 5);
        const audio_musiclayer_t *musiclayers   = params.arg[2].any;
        const u8 layercount                     = params.arg[3].u64;
        arena_t *arena                          = params.arg[4].arena;
        *(audio_music_t *)response->resource    = audio_music_init(&global_audio_engine, arena, musiclayers, layercount);
    }
}

u32 assetmanager_load_audio_music_async(assetmanager_t *const self, const audio_musiclayer_t musiclayers[AUDIO_MUSIC_LAYERS_MAX_COUNT], const u8 layer_count)
{
    const u32 asset_id = ++self->internal.asset_idx_generator;
    const asset_type type = ASSET_TYPE_MUSIC_THEME;

    taskresponse_t *response = taskresponse(self->arena, sizeof(audio_music_t));

    hashtable_insert(
        &self->assetmaps[type], 
        (hashtable_key_t){ .u32 = asset_id }, 
        response
    );

    bgtask_manager_pass_task(
        self->bgtask_manager,
        (taskrequest_t){
            .response = response,
            .on_complete = {
                .params = {
                    .count = 3,
                    .arg = {
                        [0].any = &self->internal.asset_staging_queue,
                        [1].u64 = asset_id,
                        [2].u64 = type
                    }
                },
                .callback = assetmanager__internal__notify,
            },
            .params = {
                    .count = 5,
                    .arg = {
                        [0].u64 = type,
                        [1].u64 = asset_id,
                        [2].any = (void *)musiclayers,
                        [3].u64 = layer_count,
                        [4].arena = arena_init(self->arena, layer_count * 1.5 * KB)
                    }
            },
            .storage = {0},
            .callback = assetmanager__internal__thread_callback_load_audio,
        }
    );

    return asset_id;
}

INTERNAL void assetmanager__internal__notify(const taskparams_t params)
{
    ASSERT(params.count == 3);
    mpsc_queue_t *const stagingqueue    = params.arg[0].any;
    const asset_type assetid            = params.arg[1].u64;
    const asset_type assettype          = params.arg[2].u64;

    mpsc_queue_put(
        stagingqueue,
        mem_init(
            &(asset_staging_event_t){ 
                .asset_id = assetid, 
                .type = assettype, 
                .processed_data = NULL
            }, 
            sizeof(asset_staging_event_t)
        )
    );
}

u32 assetmanager_load_audio_sfx_async(assetmanager_t *const self, const str_t filepath)
{
    const u32 asset_id = ++self->internal.asset_idx_generator;
    const asset_type assettype = ASSET_TYPE_MUSIC_SFX;

    taskresponse_t *response = taskresponse(self->arena, sizeof(audio_sfx_t));

    hashtable_insert(
        &self->assetmaps[assettype], 
        (hashtable_key_t){ .u32 = asset_id }, 
        response
    );

    bgtask_manager_pass_task(
        self->bgtask_manager,
        (taskrequest_t){
            .response = response,
            .on_complete = {
                .params = {
                    .count = 3,
                    .arg = {
                        [0].any = &self->internal.asset_staging_queue,
                        [1].u64 = asset_id,
                        [2].u64 = assettype
                    }
                },
                .callback = assetmanager__internal__notify,
            },
            .params = {
                .count = 4,
                .arg = {
                    [0].u64 = assettype,
                    [1].u64 = asset_id,
                    [2].str = filepath,
                    [3].arena = arena_init(self->arena, 1.5 * KB),
                }
            },
            .storage = {0},
            .callback = assetmanager__internal__thread_callback_load_audio,
        }
    );

    return asset_id;
}

wwise_audio_t assetmanager_load_audios_from_wwise(assetmanager_t *const self)
{
    wwise_audio_t result = {0};
    for (u32 idx = 0; idx < ARRAY_LEN(WWISE_LOADED_MUSICS); idx++)
        result.music_asset_ids[idx] = assetmanager_load_audio_music_async(self, WWISE_LOADED_MUSICS[idx], ARRAY_LEN(WWISE_LOADED_MUSICS[idx]));

    for (u32 idx = 0; idx < ARRAY_LEN(WWISE_LOADED_SFX); idx++)
        result.sfx_asset_ids[idx] = assetmanager_load_audio_sfx_async(self, WWISE_LOADED_SFX[idx]);

    logging("loaded wwise config");

    return result;
}

#endif
