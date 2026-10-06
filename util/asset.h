#pragma once
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer3d.h>

#define ASSET_MAX_TEXTURES_PER_MESH_COUNT 5 

typedef struct mesh2texture_lookup_t mesh2texture_lookup_t;
struct mesh2texture_lookup_t {
    u8 count;
    struct {
        vec4s   basecolor;
        u32     basecolor_texture_id;
        //TODO: expand this to accomadate other types of textures as needed
    } data[ASSET_MAX_TEXTURES_PER_MESH_COUNT];
};

typedef enum asset_type {

    ASSET_TYPE_MODEL                = 0,
    ASSET_TYPE_GLSL_SHADER          = 1,
    ASSET_TYPE_TEXTURE              = 2,
    ASSET_TYPE_TEXTURE_SPRITE_ATLAS = 3,
    ASSET_TYPE_MUSIC_SFX            = 4,
    ASSET_TYPE_MUSIC_THEME          = 5,
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
    u32 index_type;
    u32 attribute_count;
    struct {
        buffer_t cpu_write;
        buffer_t gpu_read;
        u8 *gpu_mem;
    } vbo;
} gpu_mesh_t;

typedef struct {
    u32 asset_id;
    struct {
        u32 count;
        gpu_mesh_t *data; 
    } meshes;
} gpu_asset_t;

typedef struct asset_staging_event_t asset_staging_event_t;
struct asset_staging_event_t {
    u32         asset_id;
    asset_type  type;
    bool        is_simulated;
    void        *processed_data;
};

#define INVALID_ASSET_ID 0

const bool ASSET_ASYNC_LOADING_SUPPORT[ASSET_TYPE_COUNT] = {
    [ASSET_TYPE_MODEL]                  = true,
    [ASSET_TYPE_MUSIC_SFX]              = true,
    [ASSET_TYPE_MUSIC_THEME]            = true,
    [ASSET_TYPE_GLSL_SHADER]            = false, //Requires opengl to compile shaders
    [ASSET_TYPE_TEXTURE]                = false,
    [ASSET_TYPE_TEXTURE_SPRITE_ATLAS]   = false,
};
