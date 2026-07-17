#pragma once
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer3d.h>

typedef enum asset_type {
    ASSET_TYPE_MODEL                = 0,
    ASSET_TYPE_GLSL_SHADER          = 1,
    ASSET_TYPE_TEXTURE              = 2,
    ASSET_TYPE_TEXTURE_SPRITE_ATLAS = 3,
    ASSET_TYPE_COUNT
} asset_type;


typedef struct {
    asset_type type;
    union {
        hashtable_t         *uniformlocs;
        vec2i_t             tile_counts;
    } meta;
    str_t filepath1;
    str_t filepath2;
} asset_meta_t;

typedef struct {
    u32 vao_id;
    u32 index_count;
    u32 attribute_count;
} gpu_mesh_t;

typedef struct {
    u32 asset_id;
    struct {
        u32 count;
        gpu_mesh_t *data; 
    } meshes;
} gpu_asset_t;

typedef struct gpu_asset__internal_upload_task_t gpu_asset__internal_upload_task_t;
struct gpu_asset__internal_upload_task_t {
    u32         asset_id;
    asset_type  type;
    void        *processed_data;
};

#define INVALID_ASSET_ID 0

const bool ASSET_ASYNC_LOADING_SUPPORT[ASSET_TYPE_COUNT] = {
    [ASSET_TYPE_MODEL]                  = true,
    [ASSET_TYPE_GLSL_SHADER]            = false, //Requires opengl to compile shaders
    [ASSET_TYPE_TEXTURE]                = false,
    [ASSET_TYPE_TEXTURE_SPRITE_ATLAS]   = false,
};

