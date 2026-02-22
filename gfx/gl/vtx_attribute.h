#pragma once

typedef enum {
    GL_VTX_ATTRIBUTE_TYPE_VTX           = 0,
    GL_VTX_ATTRIBUTE_TYPE_COLOR         = 1,
    GL_VTX_ATTRIBUTE_TYPE_UV            = 2,
    GL_VTX_ATTRIBUTE_TYPE_NORMAL        = 3,
    GL_VTX_ATTRIBUTE_TYPE_BITANGENTS    = 4,
    GL_VTX_ATTRIBUTE_TYPE_BONE_IDS      = 5,
    GL_VTX_ATTRIBUTE_TYPE_BONE_WEIGHT   = 6,
    GL_VTX_ATTRIBUTE_TYPE_COUNT
} gl_vtx_attribute_type;
