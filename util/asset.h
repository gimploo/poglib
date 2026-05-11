#pragma once
#include <poglib/basic.h>

typedef enum asset_type {
    ASSET_TYPE_MODEL    = 0,
    ASSET_TYPE_SHADER   = 1,
    ASSET_TYPE_COUNT
} asset_type;

typedef u32 asset_id;

#define INVALID_ASSET_ID        0


const bool ASSET_ASYNC_LOADING_SUPPORT[ASSET_TYPE_COUNT] = {
    [ASSET_TYPE_MODEL]      = true,
    [ASSET_TYPE_SHADER]     = false, //Requires opengl to compile shaders
};

