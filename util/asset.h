#pragma once
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer3d.h>

typedef enum asset_type {
    ASSET_TYPE_MODEL        = 0,
    ASSET_TYPE_GLSL_SHADER  = 1,
    ASSET_TYPE_COUNT
} asset_type;

typedef u32 asset_id;

typedef struct {
    u32 vao;
    u32 vbo;
    u32 ebo;
    u32 index_count;
} gpu_mesh_t;

typedef struct {
    asset_id id;
    u32 mesh_count;
    gpu_mesh_t *meshes; 
} gpu_asset_t;

typedef struct gpu_asset__internal_upload_task_t gpu_asset__internal_upload_task_t;
struct gpu_asset__internal_upload_task_t {
    asset_id    id;
    asset_type  type;
    void        *processed_data;
};


#define INVALID_ASSET_ID        0

const bool ASSET_ASYNC_LOADING_SUPPORT[ASSET_TYPE_COUNT] = {
    [ASSET_TYPE_MODEL]          = true,
    [ASSET_TYPE_GLSL_SHADER]    = false, //Requires opengl to compile shaders
};

