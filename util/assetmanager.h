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

    struct {
        u32 asset_idx_generator;
    } internal;

};

assetmanager_t      assetmanager_init(bgtask_manager_t * const taskmanager);
asset_id            assetmanager_load_model_async(assetmanager_t *self, const str_t filepath);
asset_id            assetmanager_load_glsl_shader(assetmanager_t *self, const str_t vtx, const str_t frag);
const void *        assetmanager_get_assetresource(const assetmanager_t * const self, const asset_type assettype, const u32 assetId);
void                assetmanager_destroy(assetmanager_t *const self);

#ifndef IGNORE_ASSETMANAGER_IMPLEMENTATION

assetmanager_t assetmanager_init(bgtask_manager_t * const taskmanager)
{
    arena_t arena = arena_init(NULL, 1 * MB);
    assetmanager_t result = {
        .assetmaps = {
            [ASSET_TYPE_MODEL] = hashtable_init(10, HT_KEY_TYPE_U32, async(glmodel_t), &arena),
            [ASSET_TYPE_SHADER] = hashtable_init(10, HT_KEY_TYPE_U32, glshader_t, &arena),
        },
        .bgtask_manager = taskmanager,
        .internal = {
            .asset_idx_generator = 0
        }
    };
    result.arena = arena;
    return result;
}

void assetmanager__internal_thread_callback_load_glmodel(const taskpayload_t payload, void *output_mem) 
{
    ASSERT(output_mem);
    ASSERT(payload.args.count == 1);
    const str_t filepath = payload.args.arg[0].str;
    *(glmodel_t *)output_mem = glmodel_init(filepath.data);
}

void assetmanager__internal_thread_callback_load_glshader(taskpayload_t payload, void *output_mem)
{
    ASSERT(payload.args.count == 2);
    ASSERT(payload.storage.is_ready);
    const str_t vtx_filepath = payload.args.arg[0].str;
    const str_t frag_filepath = payload.args.arg[1].str;
    *(glshader_t *)output_mem = glshader_file_init(vtx_filepath, frag_filepath, &payload.storage.arena);
}

asset_id assetmanager_load_model_async(assetmanager_t *const self, const str_t filepath)
{
    const u32 asset_id = ++self->internal.asset_idx_generator;

    //TODO: this is not cleaned up
    taskresponse_t *response = task_response(
        &self->arena, 
        sizeof(glmodel_t)
    );
    hashtable_insert(
        &self->assetmaps[ASSET_TYPE_MODEL], 
        (hashtable_key_t) {.u32 = asset_id }, 
        response
    );

    bgtask_manager_pass_task(
        self->bgtask_manager, 
        (taskconfig_t) {
            .payload = {
                .args = {
                    .count = 1,
                    .arg = {
                        [0] = filepath,
                    }
                },
                .storage = {0}
            },
            .callback = assetmanager__internal_thread_callback_load_glmodel,
        },
        response
    );

    return asset_id;
}

const void * assetmanager_get_assetresource(const assetmanager_t * const self, const asset_type assettype, const u32 assetId)
{
    ASSERT(assettype >= 0 && assettype < ASSET_TYPE_COUNT);

    if (!hashtable_has_key(&self->assetmaps[assettype], (hashtable_key_t){ .u32 = assetId })) {
        eprint("asset id `%i` is not a valid identifier for asset type `%i`", assetId, assettype);
    }

    const void *entry = hashtable_get_value(
        &self->assetmaps[assettype], 
        (hashtable_key_t) { .u32 = assetId }
    );

    //FIXME: workaround have a lookup table that tell which asset type are loaded async
    if (assettype == ASSET_TYPE_SHADER) {
        return entry;
    } else {
        taskresponse_t *obj = (taskresponse_t *)entry;
        return obj->is_done ? obj->resource : NULL;
    }
}


void assetmanager_destroy(assetmanager_t *const self)
{
    ASSERT(self);
    arena_destroy(&self->arena);
}

asset_id assetmanager_load_glsl_shader(assetmanager_t *self, const str_t vtx_filepath, const str_t frag_filepath)
{
    const u32 asset_id = ++self->internal.asset_idx_generator;

    glshader_t *shader = arena_reserve(&self->arena, sizeof(glshader_t));
    *shader = glshader_file_init(vtx_filepath, frag_filepath, &self->arena);

    hashtable_insert(
        &self->assetmaps[ASSET_TYPE_SHADER], 
        (hashtable_key_t) {.u32 = asset_id }, 
        shader
    );
    return asset_id;

    //FIXME Opengl limitation - cant run another gl context in another thread
    /*
    bgtask_manager_pass_task(
        self->bgtask_manager, 
        (taskconfig_t) {
            .payload = (taskpayload_t){
                .args ={
                    .count = 2,
                    .arg = {
                        [0] = vtx_filepath,
                        [1] = frag_filepath,
                    },
                },
                .storage = {
                    .is_ready = true,
                    .arena = arena
                }
            },
            .callback = assetmanager__internal_thread_callback_load_glshader,
        },
        response
    );
    */
}

#endif
