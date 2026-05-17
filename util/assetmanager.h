#pragma once
#include <poglib/basic.h>
#include "./asset.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/concurrency.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/gfx/model/assimp.h"

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

assetmanager_t              assetmanager_init(bgtask_manager_t *const taskmanager);
asset_id                    assetmanager_load_model_async(assetmanager_t *self, const str_t filepath);
asset_id                    assetmanager_load_glsl_shader(assetmanager_t *self, const str_t vtx_filepath, const str_t frag_filepath, const glshaderuniform_registry_t registry);
const void *                assetmanager_get_assetresource(const assetmanager_t *const self, const asset_type assettype, const u32 assetId);
void                        assetmanager_destroy(assetmanager_t *const self);


#ifndef IGNORE_ASSETMANAGER_IMPLEMENTATION

#define MAX_ASSETS_ALLOWED_PER_TYPE 10

assetmanager_t assetmanager_init(bgtask_manager_t *const taskmanager)
{
    ASSERT(taskmanager);
    arena_t arena = arena_init(NULL, 1 * MB);
    assetmanager_t result = {
        .assetmaps = {
            [ASSET_TYPE_MODEL] = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, async(glmodel_t), &arena),
            [ASSET_TYPE_GLSL_SHADER] = hashtable_init(MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, glshader_t, &arena),
        },
        .bgtask_manager = taskmanager,
        .gpu_uploaded_assets = hashtable_init(ASSET_TYPE_COUNT * MAX_ASSETS_ALLOWED_PER_TYPE, HT_KEY_TYPE_U32, gpu_asset_t, &arena),
        .internal = {
            .asset_idx_generator = 0,
            .gpu_upload_queue = mpsc_queue(&arena, MAX_ASSETS_ALLOWED_PER_TYPE * ASSET_TYPE_COUNT, sizeof(gpu_asset_t))
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

    gpu_asset__internal_upload_task_t *task = arena_reserve(&storage.arena, sizeof(gpu_asset__internal_upload_task_t));
    *task = (gpu_asset__internal_upload_task_t){
        .id             = payload.args.arg[2].u64,
        .type           = ASSET_TYPE_MODEL,
        .processed_data = output_mem,
    };

    mpsc_queue_t * const queue = (mpsc_queue_t *)payload.args.arg[1].any;
    mpsc_queue_put(queue, task, sizeof(gpu_asset__internal_upload_task_t));
}

asset_id assetmanager_load_model_async(assetmanager_t *const self, const str_t filepath)
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
                .is_ready = true
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
    arena_destroy(&self->arena);
}

asset_id assetmanager_load_glsl_shader(assetmanager_t *self, const str_t vtx_filepath, const str_t frag_filepath, const glshaderuniform_registry_t registry) 
{
    const u32 asset_id = ++self->internal.asset_idx_generator;

    glshader_t *shader = arena_reserve(&self->arena, sizeof(glshader_t));
    *shader = glshader_init(vtx_filepath, frag_filepath, registry, &self->arena);

    hashtable_insert(&self->assetmaps[ASSET_TYPE_GLSL_SHADER], (hashtable_key_t){.u32 = asset_id}, shader);
    return asset_id;
}

void assetmanager__internal_upload_model_to_gpu(const glmodel_t * const model, const asset_id asset_id)
{

}
void assetmanager__internal_upload_model_to_gpu(
    assetmanager_t *self, 
    const glmodel_t * const model, 
    const asset_id asset_id)
{
    // 1. Allocate the GPU asset container and its mesh array from the main arena
    gpu_asset_t *gpu_asset = arena_reserve(&self->arena, sizeof(gpu_asset_t));
    gpu_asset->id = asset_id;
    gpu_asset->mesh_count = model->mesh_count;
    gpu_asset->meshes = arena_reserve(&self->arena, sizeof(gpu_mesh_t) * gpu_asset->mesh_count);

    // 2. Upload each sub-mesh to VRAM
    for (u32 i = 0; i < gpu_asset->mesh_count; i++) {
        gpu_mesh_t *gpu_mesh = &gpu_asset->meshes[i];
        list_iterator(&model->meshes, iter) 
        {
            const glmesh_t *cpu_mesh = iter;
            gpu_mesh->index_count = cpu_mesh->idx.len;

            // Generate buffers
            glGenVertexArrays(1, &gpu_mesh->vao);
            glGenBuffers(1, &gpu_mesh->vbo);
            glGenBuffers(1, &gpu_mesh->ebo);

            // Bind and upload
            glBindVertexArray(gpu_mesh->vao);

            // VBO (Vertices)
            glBindBuffer(GL_ARRAY_BUFFER, gpu_mesh->vbo);
            glBufferData(GL_ARRAY_BUFFER, slot_get_size(&cpu_mesh->vtx), cpu_mesh->vtx.data, GL_STATIC_DRAW);

            // EBO (Indices)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_mesh->ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, cpu_mesh->idx.len * sizeof(u32), cpu_mesh->idx.data, GL_STATIC_DRAW);

            // Define Vertex Attributes (Positions, Normals, UVs)
            // Example for positions at layout location 0:
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glvertex3d_t), (void*)offsetof(glvertex3d_t, pos));

            // Unbind VAO to prevent accidental modification
            glBindVertexArray(0);
        }

    }

    // 3. Store the final GPU handles in the active rendering map
    hashtable_insert(&self->gpu_uploaded_assets, (hashtable_key_t){.u32 = asset_id}, gpu_asset);
}

void assetmanager_commit_gpu_uploads(assetmanager_t * const self)
{
    mpsc_queue_t *queue = &self->internal.gpu_upload_queue;

    const gpu_asset__internal_upload_task_t *task = (gpu_asset__internal_upload_task_t *)mpsc_queue_get(queue);
    switch(task->type)
    {
        case ASSET_TYPE_MODEL:
            assetmanager__internal_upload_model_to_gpu((glmodel_t *)task->processed_data, task->id);
        break;

        case ASSET_TYPE_GLSL_SHADER:
        default: eprint("asset type not implemented");
    }
}

#endif
