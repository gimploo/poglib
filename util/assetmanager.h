#pragma once
#include <poglib/basic.h>
#include "./asset.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/concurrency.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/gfx/gl/objects.h"
#include "poglib/gfx/gl/renderconfig.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/gfx/gl/types.h"
#include "poglib/gfx/gl/vbo_stream_types.h"
#include "poglib/gfx/model/assimp.h"

/*=====================================================================================================
                                -- ASSET MANAGER --
=====================================================================================================*/


typedef struct assetmanager_t assetmanager_t;
struct assetmanager_t {
    arena_t             arena;
    bgtask_manager_t    *bgtask_manager;
    hashtable_t         assetmaps[ASSET_TYPE_COUNT];
    hashtable_t         gpu_uploaded_assets;
    struct {
        u32             asset_idx_generator;
        mpsc_queue_t    gpu_upload_queue;
    } internal;
};


assetmanager_t      assetmanager_init(bgtask_manager_t *const taskmanager);
void                assetmanager_update(assetmanager_t * const self);

void                assetmanager_load_all_primitives(assetmanager_t * const self);
u32                 assetmanager_load_model_async(assetmanager_t *self, const str_t filepath);
u32                 assetmanager_load_texture(assetmanager_t *self, const str_t filepath);
u32                 assetmanager_load_glsl_shader(assetmanager_t *self, const str_t vtx_filepath, const str_t frag_filepath, const gluniform_registry_t registry);

const void *        assetmanager_get_assetresource(const assetmanager_t *const self, const asset_type assettype, const u32 assetId);
gpu_asset_t *       assetmanager_get_gpu_loaded_asset(const assetmanager_t * const self, const u32 asset_id);
gpu_mesh_t *        assetmanager_get_gpu_loaded_primitive_asset(const assetmanager_t * const self, const glmesh_primitive_type type);

void                assetmanager_destroy(assetmanager_t *const self);



#ifndef IGNORE_ASSETMANAGER_IMPLEMENTATION

#define MAX_ASSETS_ALLOWED_PER_TYPE 10

assetmanager_t assetmanager_init(bgtask_manager_t *const taskmanager)
{
    ASSERT(taskmanager);
    arena_t arena = arena_init(NULL, 1 * MB);
    assetmanager_t result = {
        .assetmaps = {
            [ASSET_TYPE_MODEL] = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(async(glmodel_t)), .type = HT_STORAGE_BY_REFERENCE }, &arena),
            [ASSET_TYPE_GLSL_SHADER] = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(glshader_t), .type = HT_STORAGE_BY_REFERENCE }, &arena),
            [ASSET_TYPE_TEXTURE] = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(gltexture2d_t),  .type = HT_STORAGE_BY_REFERENCE }, &arena),
            [ASSET_TYPE_PRIMITIVE_MESH] = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(glmesh_primitive_type), .type = HT_STORAGE_BY_REFERENCE }, &arena),
        },
        .bgtask_manager = taskmanager,
        .gpu_uploaded_assets = hashtable_init(ASSET_TYPE_COUNT * MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, (ht_value_type) { .size = sizeof(gpu_asset_t), .type = HT_STORAGE_BY_REFERENCE }, &arena),
        .internal = {
            .asset_idx_generator = 0,
            .gpu_upload_queue = mpsc_queue(&arena, MAX_ASSETS_ALLOWED_PER_TYPE * ASSET_TYPE_COUNT)
        }
    };
    result.arena = arena;
    return result;
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
    hashtable_iterator(&self->gpu_uploaded_assets, entry)
    {
        gpu_asset_t * const asset = ((hashtable_entry_t * )entry)->value;
        for (u16 idx = 0; idx < asset->meshes.count; idx++) {
            const gpu_mesh_t gpu_mesh = asset->meshes.data[idx];
            vao_destroy(&(vao_t){.id  = gpu_mesh.vao_id});
        }

    }
    arena_destroy(&self->arena);
}

u32 assetmanager_load_glsl_shader(assetmanager_t *self, const str_t vtx_filepath, const str_t frag_filepath, const gluniform_registry_t registry) 
{
    const u32 asset_id = ++self->internal.asset_idx_generator;

    glshader_t *shader = arena_reserve(&self->arena, sizeof(glshader_t));
    *shader = glshader_init(vtx_filepath, frag_filepath, registry, &self->arena);

    hashtable_insert(&self->assetmaps[ASSET_TYPE_GLSL_SHADER], (hashtable_key_t){.u32 = asset_id}, shader);
    logging("Shader compiled %s, %s", shader->fg.data, shader->vs.data);
    return asset_id;
}

void assetmanager__internal_upload_model_to_gpu(
    assetmanager_t *self, 
    const glmodel_t * const model, 
    const u32 asset_id)
{
    gpu_asset_t * const gpu_asset = arena_reserve(&self->arena, sizeof(gpu_asset_t));
    gpu_asset->asset_id = asset_id;
    gpu_asset->meshes.count = model->meshes.len;
    gpu_asset->meshes.data = arena_reserve(&self->arena, sizeof(gpu_mesh_t) * gpu_asset->meshes.count);

    const glvtx_attributelist_t attrlist = glmodel_get_attirbutelist(model);

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
                    .vertex_count = cpu_mesh->idx.len
                },
                [VBO_STREAM_TYPE_INSTANCE] = {0},
            }
        });

        ebo_init(&vbo, (u32 *)cpu_mesh->idx.data, cpu_mesh->idx.len);

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

        gpu_asset->meshes.data[(u64)list_index] = (gpu_mesh_t){
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

void assetmanager_update(assetmanager_t * const self)
{
    ASSERT(self);
    assetmanager__internal_process_pending_gpu_tasks(self);
}

void assetmanager__internal_upload_cube_to_gpu(assetmanager_t * const self)
{
    const u32 asset_id = ++self->internal.asset_idx_generator;

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
                .vertex_count = ARRAY_LEN(DEFAULT_CUBE_INDICES_24)
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
        sizeof(gpu_mesh_t));

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

    hashtable_insert(&self->assetmaps[ASSET_TYPE_PRIMITIVE_MESH], (hashtable_key_t){ .u32 = asset_id }, GL_MESH_PRIMITIVE_TYPE_CUBE);
    hashtable_insert(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id }, gpu_asset);
}


void assetmanager__internal_upload_capsule_to_gpu(assetmanager_t * const self)
{
    const u32 asset_id = ++self->internal.asset_idx_generator;

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
                .vertex_count = ARRAY_LEN(DEFAULT_CAPSULE_INDICES)
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

    hashtable_insert(&self->assetmaps[ASSET_TYPE_PRIMITIVE_MESH], (hashtable_key_t){ .u32 = asset_id }, GL_MESH_PRIMITIVE_TYPE_CAPSULE);
    hashtable_insert(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id }, gpu_asset);
}


gpu_mesh_t * assetmanager_get_gpu_loaded_primitive_asset(const assetmanager_t * const self, const glmesh_primitive_type type)
{
    hashtable_iterator(&self->assetmaps[ASSET_TYPE_PRIMITIVE_MESH], iter)
    {
        const hashtable_entry_t *entry = iter;
        if ((glmesh_primitive_type)entry->value == type) {
            const gpu_asset_t * const asset = assetmanager_get_gpu_loaded_asset(self, entry->key.u32);
            ASSERT(asset->meshes.count > 0);
            return asset->meshes.data;
        }
    }
    eprint("primitive type mesh found -- investigate!");
}


void assetmanager_load_all_primitives(assetmanager_t *const self)
{
    assetmanager__internal_upload_cube_to_gpu(self);
    assetmanager__internal_upload_capsule_to_gpu(self);
}

gpu_asset_t * assetmanager_get_gpu_loaded_asset(const assetmanager_t * const self, const u32 asset_id)
{
    return (gpu_asset_t *)hashtable_get_value(&self->gpu_uploaded_assets, (hashtable_key_t){ .u32 = asset_id });
}

#endif
