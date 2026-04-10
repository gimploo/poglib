#pragma once
#include "poglib/gfx/gl/vbo_stream_types.h"
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/gfx/gl/shader.h>
#include <poglib/gfx/gl/texture2d.h>
#include <poglib/gfx/gl/texture_types.h>
#include <poglib/gfx/gl/cubemap.h>

#define MAX_SUPPORTED_TEXTURE_COUNT_PER_DRAW_CALL 64
#define MAX_ATTRIBUTES_COUNT_PER_CALL 16

typedef struct {
    union {
        gltexture2d_t *normal_texture;
        glcubemap_t *cubemap;
    } source;
    gltexturetype type; 
} gltextureitem_t;

typedef struct {
    u8 count;
    gltextureitem_t items[MAX_SUPPORTED_TEXTURE_COUNT_PER_DRAW_CALL];
} gltexturelist_t;

typedef struct {
    u8 ncmp;
    u32 type;                           //NOTE: Attributes are only handelled for GL_FLOAT (default) and GL_INT
    vbo_stream_type vbo_chunk_index;    //NOTE: Defaults to VBO_GEOMETRY
    struct {
        u32 offset;
        u32 stride;
    } interleaved;
} glvtx_attribute_t;


typedef struct {
    u8 count;
    glvtx_attribute_t attr[MAX_ATTRIBUTES_COUNT_PER_CALL];
} glvtx_attributelist_t;


typedef struct {
    const char *name;
    const char *type;
    union {
        matrix4f_t  mat4;
        vec4f_t     vec4;
        vec3f_t     vec3;
        vec2f_t     vec2;
        f32         f32;
        i32         i32;
        u32         u32;
        bool        boolean;
        struct {
            matrix4f_t *data;
            u32 count;
        } mat4s;
    } value;
} uniform_t;

typedef struct {
    u8 count;
    uniform_t uniform[10];
} gluniforms_t;

typedef struct {
    const glshader_t *shader;
    gluniforms_t uniforms;
} glshaderconfig_t;

typedef struct {
    u32 count;
    glshaderconfig_t configs[3];
} glshaderconfiglist_t;
