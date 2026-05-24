#pragma once
#include "common.h"
#include "poglib/basic/common.h"
#include "poglib/gfx/gl/vbo_stream_types.h"

#define GL_VAO_INVALID_ID     0
#define GL_VBO_INVALID_ID     0
#define GL_EBO_INVALID_ID     0

typedef struct { 
    u32 id; 
} vao_t ;

typedef struct {
    union {
        u32  vertex_count;
        u32  instance_count;
    };
    buffer_t    buffer;
} vbo_stream_t;

typedef struct {
    //NOTE: GL_STATIC_DRAW / GL_DYNAMIC_DRAW
    u32             usage; 
    vbo_stream_t    chunks[VBO_STREAM_TYPE_COUNT];
} vbo_config_t;

typedef struct {
    u32  id; 
    struct {
        vbo_config_t config;
        i64     attribute_index;
    } internals;
} vbo_t ;

typedef struct ebo_t {

    u32  id;
    u32  indices_count;
    vbo_t *vbo;

} ebo_t ;


vbo_t           vbo_init(const vbo_config_t config);

//FIXME: @deprecated
vbo_t           vbo_static_init(const void *vertices, const size_t vsize, const u64 count);

#define         vbo_bind(PVBO)                                  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, (PVBO)->id))
#define         vbo_unbind()                                    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0))
void            vbo_destroy(const vbo_t *);


ebo_t           ebo_init(vbo_t *vbo, const u32 *array, const u32 nmemb);
#define         ebo_bind(PEBO)  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (PEBO)->id))
void            ebo_destroy(const ebo_t *ebo);


vao_t           vao_init(void);
#define         vao_bind(PVAO)                                   GL_CHECK(glBindVertexArray((PVAO)->id))
void            vao_set_attributes( vao_t *vao, vbo_t *vbo, u8 component_count, GLenum type, bool normalized, size_t stride, size_t offset, const u8 instance_divisor, const vbo_stream_type chunk_type);
#define         vao_unbind()                                    GL_CHECK(glBindVertexArray(0))
#define         vao_draw_with_vbo(PVAO, PVBO)                   __impl_vao_draw_with_vbo(PVAO, PVBO, GL_TRIANGLES)
#define         vao_draw_with_vbo_in_mode(PVAO, PVBO, MODE)     __impl_vao_draw_with_vbo(PVAO, PVBO, MODE)
void            vao_draw_with_vbo_in_mode_instanced(const vao_t *vao, const vbo_t *vbo, u64 gldraw_mode);
#define         vao_draw_with_ebo(PVAO, PEBO)                   __impl_vao_draw_with_ebo(PVAO, PEBO, GL_TRIANGLES)
#define         vao_draw_with_ebo_in_mode(PVAO, PEBO, MODE)     __impl_vao_draw_with_ebo(PVAO, PEBO, MODE)
void            vao_draw_with_ebo_in_mode_instanced(const vao_t *vao, const ebo_t *ebo, const u64 gldraw_mode);

void            vao_destroy(const vao_t *vao);




#ifndef IGNORE_GL_OBJECTS_IMPLEMENTATION

//vbo

void vbo_destroy(const vbo_t *obj)
{
    if (obj == NULL) eprint("vbo_destroy: obj argument is null\n");

    GL_LOG("VBO `%i` deleted", obj->id);
    GL_CHECK(glDeleteBuffers(1, &obj->id));
}


vbo_t vbo_static_init(
    const void *vertices,
    const size_t vsize,
    const u64 vertex_count)
{
    ASSERT(vsize != 8);
    vbo_config_t config = {
        .usage = GL_STATIC_DRAW,
        .chunks = {
            [VBO_STREAM_TYPE_GEOMETRY] = {
                .vertex_count = vertex_count,
                .buffer = {
                    .raw_data = (u8 *)vertices,
                    .size = vsize,
                }
            }
        }
    };

    vbo_t VBO = {
        .internals = {
            .config = config,
            .attribute_index = -1,
        }
    };

    GL_CHECK(glGenBuffers(1, &VBO.id)); 
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO.id));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vsize, vertices, GL_STATIC_DRAW));
    if (vertices != NULL) {
        ASSERT(vertex_count > 0);
        GL_LOG("VBO (STATIC)\t `%i` created", VBO.id);
    } else {
        GL_LOG("EMPTY VBO (STATIC)\t `%i` created", VBO.id);
    }

    return VBO;
}

vbo_t vbo_init(const vbo_config_t config)
{
    u32 vbo_id;

    u32 total_buffer_size = 0;
    for(u32 idx = 0; idx < VBO_STREAM_TYPE_COUNT; idx++) {
        total_buffer_size += config.chunks[idx].buffer.size;
    }
    ASSERT(total_buffer_size > 0);

    GL_CHECK(glGenBuffers(1, &vbo_id)); 
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo_id));

    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, total_buffer_size, NULL, config.usage));

    for (u32 idx = 0, offset = 0; idx < VBO_STREAM_TYPE_COUNT; idx++)
    {
        GL_CHECK(glBufferSubData(
            GL_ARRAY_BUFFER, 
            offset,
            config.chunks[idx].buffer.size, 
            config.chunks[idx].buffer.raw_data)
        );
        offset += config.chunks[idx].buffer.size;
    }

    return (vbo_t) {
        .id = vbo_id,
        .internals = {
            .attribute_index = -1,
            .config = config
        }
    };
}

//ebo

ebo_t ebo_init(vbo_t *vbo, const u32 *indices, const u32 nindices)
{
    if (indices == NULL) eprint("ebo_init: indices argument is null");
    if (nindices <= 0) eprint("ebo_init: isize is not greater than 0");

    ASSERT(vbo);

    ebo_t ebo = {
        .vbo = vbo,
        .indices_count = nindices,
    };

    GL_CHECK(glGenBuffers(1, &ebo.id));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, nindices * sizeof(u32), indices, GL_STATIC_DRAW));


    GL_LOG("EBO `%i` created", ebo.id);
    return ebo;
}

void ebo_destroy(const ebo_t *ebo)
{
    if (ebo == NULL) eprint("ebo_bind: ebo is null argument");

    GL_LOG("EBO `%i` deleted", ebo->id);
    GL_CHECK(glDeleteBuffers(1, &ebo->id));
}

//vao

vao_t vao_init(void)
{
    vao_t vao;

    GL_CHECK(glGenVertexArrays(1, &vao.id)); 

    GL_LOG("VAO `%i` created", vao.id);

    return vao;
}

void vao_set_attributes(
            vao_t *vao,
            vbo_t *vbo,
            u8 component_count, 
            GLenum type,
            bool normalized,
            size_t stride,
            size_t offset,
            const u8 instance_divisor, //NOTE: Default to zero if the attribute is to be applied for each vertex,
            const vbo_stream_type chunk_type //NOTE: Default is VBO_STREAM_TYPE_GEOMETRY
) {
    if (vao == NULL) eprint("vao_set_attribute: vao argument is null");
    if (vbo == NULL) eprint("vbo argument is null");

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo->id));
    GL_CHECK(glEnableVertexAttribArray(++vbo->internals.attribute_index));

    const u64 geometry_chunk_size = vbo->internals.config.chunks[VBO_STREAM_TYPE_GEOMETRY].buffer.size;
    const u64 buffer_offset = chunk_type == VBO_STREAM_TYPE_INSTANCE ? geometry_chunk_size + offset : offset;

    switch(type)
    {
        case GL_INT:
            GL_CHECK(glVertexAttribIPointer(
                        vbo->internals.attribute_index,
                        component_count, 
                        GL_INT,
                        stride,
                        (const void *)buffer_offset));
        break;
        case GL_FLOAT:
            GL_CHECK(glVertexAttribPointer(
                        vbo->internals.attribute_index,
                        component_count, 
                        GL_FLOAT, 
                        normalized == false ? GL_FALSE : GL_TRUE, 
                        stride, 
                        (const void *)buffer_offset));
        break;
        default: eprint("Not implemented");
    }
    GL_CHECK(glVertexAttribDivisor(vbo->internals.attribute_index, instance_divisor));
}

void vao_draw_with_vbo_in_mode_instanced(const vao_t *vao, const vbo_t *vbo, u64 gldraw_mode)
{
    if (vao == NULL) eprint("vao_draw: vao argument is null");

    const u32 vtx_count = vbo->internals.config.chunks[VBO_STREAM_TYPE_GEOMETRY].vertex_count;
    const u32 instance_count = vbo->internals.config.chunks[VBO_STREAM_TYPE_INSTANCE].instance_count;

    if (!vtx_count) eprint("vao_draw: vbo`s vertex_count is %lu", vtx_count);
    if (!instance_count) eprint("vao_draw: vbo`s instance_count is %lu", instance_count);

    GL_CHECK(glDrawArraysInstanced(
                gldraw_mode, 
                0, 
                vbo->internals.config.chunks[VBO_STREAM_TYPE_GEOMETRY].vertex_count,
                vbo->internals.config.chunks[VBO_STREAM_TYPE_INSTANCE].vertex_count));

    GL_CHECK(glBindVertexArray(0));
}

void __impl_vao_draw_with_vbo(const vao_t *vao, const vbo_t *vbo, u64 gldraw_mode)
{
    if (vao == NULL) eprint("vao_draw: vao argument is null");

    const u32 vtx_count = vbo->internals.config.chunks[VBO_STREAM_TYPE_GEOMETRY].vertex_count;

    if (!vtx_count) eprint("vao_draw: vbo`s vertex_count is %lu", vtx_count);

    GL_CHECK(glDrawArrays(gldraw_mode, 0, vtx_count));
    GL_CHECK(glBindVertexArray(0));
}

void __impl_vao_draw_with_ebo(const vao_t *vao, const ebo_t *ebo, const u64 gldraw_mode)
{
    if (vao == NULL) eprint("vao_draw: vao argument is null");

    if (ebo->indices_count == 0) eprint("vao_draw: vbo`s indices_count is %i", ebo->indices_count);

    GL_CHECK(glDrawElements(
                gldraw_mode, 
                ebo->indices_count, 
                GL_UNSIGNED_INT, 
                0));
    GL_CHECK(glBindVertexArray(0));
}

void vao_draw_with_ebo_in_mode_instanced(const vao_t *vao, const ebo_t *ebo, const u64 gldraw_mode)
{
    if (vao == NULL) eprint("vao_draw: vao argument is null");

    if (ebo->indices_count == 0) eprint("vao_draw: vbo`s indices_count is %i", ebo->indices_count);

    const u32 instance_count = ebo->vbo->internals.config.chunks[VBO_STREAM_TYPE_INSTANCE].instance_count;

    GL_CHECK(glDrawElementsInstanced(
                gldraw_mode, 
                ebo->indices_count, 
                GL_UNSIGNED_INT, 
                0,
                instance_count));

    GL_CHECK(glBindVertexArray(0));
}


void vao_destroy(const vao_t *vao)
{
    if (vao == NULL) eprint("vao_bind: vao argument is null");

    GL_LOG("VAO `%i` deleted", vao->id);
    GL_CHECK(glDeleteVertexArrays(1, &vao->id));
}

#endif
