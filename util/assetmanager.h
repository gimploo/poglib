#pragma once
#include <poglib/basic.h>
#include "./asset.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/concurrency.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/basic/str.h"
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
    arena_t             arena;
    bgtask_manager_t    *bgtask_manager;
    hashtable_t         assetmaps[ASSET_TYPE_COUNT];
    hashtable_t         gpu_uploaded_assets;
    hashtable_t         assetmeta_lookup;
    struct {
        u32             asset_idx_generator;
        mpsc_queue_t    gpu_upload_queue;
    } internal;
};


assetmanager_t      assetmanager_init(bgtask_manager_t *const taskmanager);
void                assetmanager_update(assetmanager_t *const self);

void                assetmanager_load_all_primitives(assetmanager_t * const self);
u32                 assetmanager_load_model_async(assetmanager_t *self, const str_t filepath);
u32                 assetmanager_load_spriteatlas(assetmanager_t *const self, const str_t filepath, const u32 tile_count_width, const u32 tile_count_height);
u32                 assetmanager_load_texture(assetmanager_t *self, const str_t filepath);
u32                 assetmanager_load_glsl_shader(assetmanager_t *self, const str_t vtx_filepath, const str_t frag_filepath, const gluniform_registry_t registry);

const void *        assetmanager_get_assetresource(const assetmanager_t *const self, const asset_type assettype, const u32 assetId);
gpu_asset_t *       assetmanager_get_gpu_loaded_asset_async(const assetmanager_t *const self, const u32 asset_id);
void                assetmanager_write_assetmeta_data_to_file(const assetmanager_t *const self, file_t *const file);

void                assetmanager_destroy(assetmanager_t *const self);



#ifndef IGNORE_ASSETMANAGER_IMPLEMENTATION

#define MAX_ASSETS_ALLOWED_PER_TYPE 10

INTERNAL void assetmanager__internal_add_asset_meta_data(assetmanager_t *const self, const asset_type type, const u32 asset_id, const str_t filepath1, const str_t filepath2, void *const asset_data);
INTERNAL void assetmanager__internal_write_uniformlocs_to_file(const hashtable_entry_t *const entry, buffer_t *const buffer);

assetmanager_t assetmanager_init(bgtask_manager_t *const taskmanager)
{
    ASSERT(taskmanager);
    arena_t arena = arena_init(NULL, 1 * MB);
    assetmanager_t result = {
        .assetmaps = {
            [ASSET_TYPE_MODEL] = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(async(glmodel_t)), .type = HT_STORAGE_BY_REFERENCE }, &arena),
            [ASSET_TYPE_GLSL_SHADER] = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(glshader_t), .type = HT_STORAGE_BY_REFERENCE }, &arena),
            [ASSET_TYPE_TEXTURE] = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(gltexture2d_t),  .type = HT_STORAGE_BY_REFERENCE }, &arena),
            [ASSET_TYPE_TEXTURE_SPRITE_ATLAS] = hashtable_init(3, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(spriteatlas_t),  .type = HT_STORAGE_BY_VALUE }, &arena),
        },
        .bgtask_manager = taskmanager,
        .assetmeta_lookup = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE * ASSET_TYPE_COUNT, HT_KEY_TYPE_U32, (ht_value_type){ .size = sizeof(asset_meta_t), .type = HT_STORAGE_BY_VALUE } , &arena),
        .gpu_uploaded_assets = hashtable_init(ASSET_TYPE_COUNT * MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(gpu_asset_t), .type = HT_STORAGE_BY_REFERENCE }, &arena),
        .internal = {
            .asset_idx_generator = GL_MESH_PRIMITIVE_TYPE_COUNT,
            .gpu_upload_queue = mpsc_queue(&arena, MAX_ASSETS_ALLOWED_PER_TYPE * ASSET_TYPE_COUNT)
        }
    };
    result.arena = arena;
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
                case ASSET_TYPE_MODEL: 
                    //FIXME: this doesnt work!
                    //glmodel_destroy(((taskresponse_t *)entry->value)->resource); 
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

void assetmanager__internal_thread_callback_load_glmodel(const taskpayload_t payload, taskstorage_t storage, void *output_mem) 
{
    ASSERT(output_mem);
    ASSERT(payload.args.count == 3);

    const str_t filepath = payload.args.arg[0].str;
    *(glmodel_t *)output_mem = glmodel_init(filepath.data);

    gpu_asset__internal_upload_task_t *task = arena_store(
        &storage.arena, 
        &(gpu_asset__internal_upload_task_t){
            .asset_id       = payload.args.arg[2].u64,
            .type           = ASSET_TYPE_MODEL,
            .processed_data = output_mem,
        },
        sizeof(gpu_asset__internal_upload_task_t));

    mpsc_queue_t * const queue = (mpsc_queue_t *)payload.args.arg[1].any;
    mpsc_queue_put(queue, task);
}

u32 assetmanager_load_model_async(assetmanager_t *const self, const str_t filepath)
{
    const u32 asset_id = ++self->internal.asset_idx_generator;

    taskresponse_t *response = taskresponse(&self->arena, sizeof(glmodel_t));
    hashtable_insert(
        &self->assetmaps[ASSET_TYPE_MODEL], 
        (hashtable_key_t){ .u32 = asset_id }, 
        response
    );
    assetmanager__internal_add_asset_meta_data(self, ASSET_TYPE_MODEL, asset_id, filepath, STR_EMPTY, NULL);

    bgtask_manager_pass_task(
        self->bgtask_manager,
        (taskconfig_t){
            .result_dest = response,
            .payload = {
                .args = {
                    .count = 3,
                    .arg = {
                        [0].str = filepath,
                        [1].any = &self->internal.gpu_upload_queue,
                        [2].u64 = asset_id
                    }
                },
            },
            .storage = {
                .arena = arena_init(&self->arena, sizeof(gpu_asset__internal_upload_task_t)),
            },
            .callback = assetmanager__internal_thread_callback_load_glmodel,
        }
    );

    return asset_id;
}

const void * assetmanager_get_assetresource(const assetmanager_t *const self, const asset_type assettype, const u32 assetId)
{
    ASSERT(assettype >= 0 && assettype < ASSET_TYPE_COUNT);

    if (!hashtable_has_key(&self->assetmaps[assettype], (hashtable_key_t){ .u32 = assetId })) {
        eprint("asset id `%i` is not a valid identifier for asset type `%i`", assetId, assettype);
    }

    const void *entry = hashtable_get_value(
        &self->assetmaps[assettype], 
        (hashtable_key_t){ .u32 = assetId }
    );

    const taskresponse_t *response = (taskresponse_t *)entry;
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
    arena_destroy(&self->arena);
}

u32 assetmanager_load_glsl_shader(assetmanager_t *const self, const str_t vtx_filepath, const str_t frag_filepath, const gluniform_registry_t registry) 
{
    const u32 asset_id = ++self->internal.asset_idx_generator;

    glshader_t *shader = arena_reserve(&self->arena, sizeof(glshader_t));
    *shader = glshader_init(vtx_filepath, frag_filepath, registry, &self->arena);

    assetmanager__internal_add_asset_meta_data(self, ASSET_TYPE_GLSL_SHADER, asset_id, vtx_filepath, frag_filepath, shader);
    hashtable_insert(&self->assetmaps[ASSET_TYPE_GLSL_SHADER], (hashtable_key_t){.u32 = asset_id}, shader);
    logging("Shader compiled %s, %s", shader->fg.data, shader->vs.data);
    return asset_id;
}

void assetmanager__internal_upload_model_to_gpu(
    assetmanager_t *const self,
    const glmodel_t *const model,
    const u32 asset_id
) {
    gpu_asset_t * const gpu_asset           = arena_reserve(&self->arena, sizeof(gpu_asset_t));
    gpu_asset->asset_id                     = asset_id;
    gpu_asset->meshes.count                 = model->meshes.len;
    gpu_asset->meshes.data                  = arena_reserve(&self->arena, sizeof(gpu_mesh_t) * gpu_asset->meshes.count);

    list_iterator(&model->meshes, iter) 
    {
        const glmesh_t * const cpu_mesh = iter;

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

    logging("Model %s loaded to GPU", model->filepath);

    hashtable_insert(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id }, gpu_asset);
}

void assetmanager__internal_process_pending_gpu_tasks(assetmanager_t * const self)
{
    mpsc_queue_t *queue = &self->internal.gpu_upload_queue;

    while(true) {
        const gpu_asset__internal_upload_task_t *task = (gpu_asset__internal_upload_task_t *)mpsc_queue_get(queue);
        if (!task) {
            return;
        }

        switch(task->type)
        {
            case ASSET_TYPE_MODEL:
                assetmanager__internal_upload_model_to_gpu(self, (glmodel_t *)task->processed_data, task->asset_id);
            break;

            case ASSET_TYPE_TEXTURE:
            case ASSET_TYPE_GLSL_SHADER:
            default: eprint("asset type async gpu loading functions are not implemented");
        }
    }
}

void assetmanager_update(assetmanager_t *const self)
{
    ASSERT(self);
    assetmanager__internal_process_pending_gpu_tasks(self);
}

void assetmanager__internal_upload_cube_to_gpu(assetmanager_t *const self)
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
        &self->arena, 
        &(gpu_mesh_t) {
            .vao_id = vao.id,
            .index_count = ebo.indices_count,
            .attribute_count = vbo.internals.attribute_index + 1
        },
        sizeof(gpu_mesh_t)
    );

    gpu_asset_t * const gpu_asset = arena_store(
        &self->arena, 
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


void assetmanager__internal_upload_capsule_to_gpu(assetmanager_t *const self)
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
        &self->arena, 
        &(gpu_mesh_t) {
            .vao_id = vao.id,
            .index_count = ebo.indices_count,
            .attribute_count = vbo.internals.attribute_index + 1
        },
        sizeof(gpu_mesh_t));

    const gpu_asset_t * const gpu_asset = arena_store(
        &self->arena, 
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


void assetmanager__internal_upload_camera_model(assetmanager_t *const self)
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
        &self->arena, 
        &(gpu_mesh_t) {
            .vao_id = vao.id,
            .index_count = ebo.indices_count,
            .attribute_count = vbo.internals.attribute_index + 1
        },
        sizeof(gpu_mesh_t));

    const gpu_asset_t *const gpu_asset = arena_store(
        &self->arena, 
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

void assetmanager__internal_upload_line_to_gpu(assetmanager_t *const self)
{
    const u32 asset_id = GL_MESH_PRIMITIVE_TYPE_LINE;

    vao_t vao = vao_init();
    vao_bind(&vao);

    const f32 DEFAULT_LINE_VERTICES[] = {
        0.0f, 0.0f, 0.0f,   // start
        1.0f, 0.0f, 0.0f,   // end
    };

    const u32 DEFAULT_LINE_INDICES[] = {
        0, 1,
    };

    vbo_t vbo = vbo_init((vbo_config_t) {
        .usage = GL_STATIC_DRAW,
        .chunks = {
            [VBO_STREAM_TYPE_GEOMETRY] = { 
                .buffer = {
                    .raw_data = (u8 *)DEFAULT_LINE_VERTICES,
                    .size = sizeof(DEFAULT_LINE_VERTICES),
                }, 
                .index_count = ARRAY_LEN(DEFAULT_LINE_INDICES)
            },
            [VBO_STREAM_TYPE_INSTANCE] = {0},
        }
    });

    const ebo_t ebo = ebo_init(&vbo, DEFAULT_LINE_INDICES, ARRAY_LEN(DEFAULT_LINE_INDICES));

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

    gpu_mesh_t * const gpu_mesh = arena_store(
        &self->arena, 
        &(gpu_mesh_t) {
            .vao_id = vao.id,
            .index_count = ebo.indices_count,
            .attribute_count = vbo.internals.attribute_index + 1
        },
        sizeof(gpu_mesh_t));

    const gpu_asset_t * const gpu_asset = arena_store(
        &self->arena, 
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
    assetmanager__internal_upload_line_to_gpu(self);
    assetmanager__internal_upload_cube_to_gpu(self);
    assetmanager__internal_upload_capsule_to_gpu(self);
    assetmanager__internal_upload_camera_model(self);
}

gpu_asset_t * assetmanager_get_gpu_loaded_asset_async(const assetmanager_t *const self, const u32 asset_id)
{
    return (gpu_asset_t *)hashtable_get_value_or_null(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id });
}

u32 assetmanager_load_spriteatlas(assetmanager_t *const self, const str_t filepath, const u32 tile_count_width, const u32 tile_count_height)
{
    const spriteatlas_t atlas   = spriteatlas_init(filepath, tile_count_width, tile_count_height, &self->arena);
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
        .meta = {0}
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
                "assetid:%u,assetpath:[%.*s,%.*s]\n",
                entry->key.u32,
                assetmeta->filepath1.len,
                assetmeta->filepath1.data,
                assetmeta->filepath2.len,
                assetmeta->filepath2.data
            );
        else if (!assetmeta->filepath1.len && assetmeta->filepath2.len)
            snprintf(
                buffer.raw_data, sizeof(buffer.raw_data), 
                "assetid:%u,assetpath:%.*s\n",
                entry->key.u32,
                assetmeta->filepath2.len,
                assetmeta->filepath2.data
            );
        else if (assetmeta->filepath1.len && !assetmeta->filepath2.len)
            snprintf(
                buffer.raw_data, sizeof(buffer.raw_data), 
                "assetid:%u,assetpath:%.*s\n",
                entry->key.u32,
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
                    "\ttilecount:[%u,%u]\n",
                    assetmeta->meta.tile_counts.x,
                    assetmeta->meta.tile_counts.y
                );
            break;
            case ASSET_TYPE_GLSL_SHADER:
                hashtable_serialize_to_file(
                    assetmeta->meta.uniformlocs, 
                    file,
                    assetmanager__internal_write_uniformlocs_to_file
                );
            break;

        }
        file_writeline(file, buffer.raw_data);
    }
}

void assetmanager__internal_write_uniformlocs_to_file(const hashtable_entry_t *const entry, buffer_t *const buffer)
{
    gluniform__internal_meta_t *uniformmeta = entry->value;
    snprintf(buffer->raw_data, buffer->size, 
        "\tuniform:\n"
        "\t\tname:%.*s,type:%u\n",
        uniformmeta->name.len, uniformmeta->name.data, uniformmeta->type
    );
}



#endif
