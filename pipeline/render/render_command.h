#pragma once
#include "poglib/basic/arena.h"
#include "poglib/basic/ds/list.h"
#include "poglib/gfx/gl/common.h"
#include "poglib/gfx/gl/renderconfig.h"
#include "poglib/gfx/gl/types.h"
#include "poglib/pipeline/render/common.h"
#include <poglib/gfx/glrenderer3d.h>

typedef enum {
    RENDER_COMMAND_DRAW_MODE_TRIANGLE = GL_TRIANGLES,
    RENDER_COMMAND_DRAW_MODE_LINES = GL_LINES,
    RENDER_COMMAND_DRAW_MODE_COUNT,
} render_command_draw_mode;

typedef struct {
    render_command_types        type;
    render_command_draw_mode    draw_mode;
    union {

        struct {
            matrix4f_t camera_view;
            matrix4f_t projection;
            vec3f_t translation;
        } instance_config;

        struct {
            buffer_t                vtx[VBO_STREAM_TYPE_COUNT];
            struct {
                u8 nmemb;
                u8 *data;
            }                       idx;
            struct {
                u8                  len;
                glvtx_attribute_t   *data;
            } attrs;
            gltexturelist_t         textures;
            glshaderconfig_t        shader_config;
        } call_config;
    };

} render_command_t;

bool            render_command_are_all_attrs_the_same(const render_command_t *render_command, const render_command_t *command);
bool            render_command_are_all_textures_the_same(const render_command_t *render_command, const render_command_t *command);
buffer_t        render_command_get_vtx_buffer(const list_t *const commands, arena_t * const arena);

#ifndef IGNORE_RENDER_COMMAND_IMPLEMENTATION

buffer_t __render_command_merge_all_vtx_together(const list_t * const commands, arena_t * const arena);

bool render_command_are_all_attrs_the_same(const render_command_t *render_command, const render_command_t *command)
{
    ASSERT(render_command);
    ASSERT(command);

    if (command->call_config.attrs.len != render_command->call_config.attrs.len)
        return false;

    for (u32 idx = 0; idx < command->call_config.attrs.len; idx++)
    {
        const glvtx_attribute_t *attr_x = &command->call_config.attrs.data[idx];
        const glvtx_attribute_t *attr_y = &render_command->call_config.attrs.data[idx];

        if (attr_x->ncmp != attr_y->ncmp) return false;
        if (attr_x->type != attr_y->type) return false;
        if (attr_x->interleaved.offset != attr_y->interleaved.offset) return false;
        if (attr_x->interleaved.stride != attr_y->interleaved.stride) return false;
    }

    return true;
}

bool render_command_are_all_textures_the_same(const render_command_t *render_command, const render_command_t *command)
{
    ASSERT(render_command);
    ASSERT(command);

    if (command->call_config.textures.count != render_command->call_config.textures.count)
        return false;

    for (u8 idx = 0; idx < command->call_config.textures.count; idx++)
    {
        if (command->call_config.textures.items[idx].source.normal_texture->id != render_command->call_config.textures.items[idx].source.normal_texture->id)
            return false;
    }
    return true;
}

glshaderconfig_t render_command_get_shaderconfig(const list_t * const commands, const render_queue_t * const queue)
{
    ASSERT(commands);
    ASSERT(commands->len);
    render_command_t *bucket_type = list_get_value(commands, 0);

    const bool is_instanced = bucket_type->type & (RENDER_COMMAND_TYPE_CAPSULE | RENDER_COMMAND_TYPE_CUBE);
    if (!is_instanced) {
        return bucket_type->call_config.shader_config;
    }

    return (glshaderconfig_t) {
        .shader = &queue->internal.instance_shader,
        .uniforms = {
            .count = 2,
            .uniform = {
                [0] = {
                    .name = "projection",
                    .type = "matrix4f_t",
                    .value = bucket_type->instance_config.projection,
                },
                [1] = {
                    .name = "view",
                    .type = "matrix4f_t",
                    .value = bucket_type->instance_config.camera_view,
                }
            }
        }
    };

}

glvtx_attributelist_t render_command_get_attrs(const list_t * const commands)
{
    ASSERT(commands);
    ASSERT(commands->len);

    glvtx_attributelist_t list = {0};
    render_command_t *bucket_type = list_get_value(commands, 0);

    switch(bucket_type->type)
    {
        case RENDER_COMMAND_TYPE_CUBE: 
        case RENDER_COMMAND_TYPE_CAPSULE: 
            list.count = 3;
            //POSITION
            list.attr[0] = (glvtx_attribute_t){
                .vbo_chunk_index = VBO_STREAM_TYPE_GEOMETRY,
                .ncmp = 3,
                .interleaved = {
                    .offset = 0,
                    .stride = sizeof(f32) * 6 //INFO: vtx (3) + normals (3) 
                },
                .type = GL_FLOAT
            };
            //NORMALS
            list.attr[1] = (glvtx_attribute_t){
                .vbo_chunk_index = VBO_STREAM_TYPE_GEOMETRY,
                .ncmp = 3,
                .interleaved = {
                    .offset = sizeof(f32) * 3,
                    .stride = sizeof(f32) * 6 //INFO: vtx (3) + normals (3) 
                },
                .type = GL_FLOAT
            };
            //TRANSLATE (vec3)
            list.attr[2] = (glvtx_attribute_t){
                .ncmp = 3,
                .vbo_chunk_index = VBO_STREAM_TYPE_INSTANCE,
                .interleaved = {
                    .offset = 0,
                    .stride = sizeof(f32) * 3
                },
                .type = GL_FLOAT
            };
        break;
        case RENDER_COMMAND_TYPE_CUSTOM: 
            eprint("TODO: for custom idx");
        break;
        default: eprint("unknown render type");
    }
    return list;
}

buffer_t render_command_get_idx_buffer(const list_t * const commands, arena_t * const arena)
{
    ASSERT(commands);
    ASSERT(commands->len);
    ASSERT(arena);

    const render_command_t *command = list_get_value(commands, 0);

    buffer_t buffer = {0};
    switch(command->type)
    {
        case RENDER_COMMAND_TYPE_CUBE: 
            buffer = (buffer_t){
                .raw_data = (u8 *)DEFAULT_CUBE_INDICES_24,
                .size = sizeof(DEFAULT_CUBE_INDICES_24)
            };
        break;
        case RENDER_COMMAND_TYPE_CAPSULE: 
            buffer = (buffer_t){
                .raw_data = (u8 *)DEFAULT_CAPSULE_INDICES,
                .size = sizeof(DEFAULT_CAPSULE_INDICES)
            };
        break;
        case RENDER_COMMAND_TYPE_CUSTOM: 
            eprint("TODO: for custom idx");
        break;
        default: eprint("unknown render type");
    }

    return buffer;
}

buffer_t render_command_get_instance_buffer(const list_t * const commands, arena_t * const arena)
{
    ASSERT(commands);
    ASSERT(commands->len);
    ASSERT(arena);

    const render_command_t *command = list_get_value(commands, 0);

    buffer_t buffer = {
        .raw_data = (u8 *)arena_reserve_array(arena, vec3f_t, commands->len),
        .size = sizeof(vec3f_t) * commands->len
    };

    switch(command->type)
    {
        case RENDER_COMMAND_TYPE_CUBE: 
        case RENDER_COMMAND_TYPE_CAPSULE:
            list_iterator(commands, iter) {
                const vec3f_t translation = ((render_command_t *)iter)->instance_config.translation;
                ((vec3f_t *)buffer.raw_data)[(u64)list_index] = translation;
            }
        break;
        case RENDER_COMMAND_TYPE_CUSTOM_WITH_INSTANCING: 
            eprint("not implemented");
        break;
        default: eprint("unknown render type");
    }

    return buffer;
}


buffer_t render_command_get_vtx_buffer(const list_t * const commands, arena_t * const arena)
{
    ASSERT(commands);
    ASSERT(commands->len);
    ASSERT(arena);

    const render_command_t *command = list_get_value(commands, 0);

    buffer_t buffer = {0};
    switch(command->type)
    {
        case RENDER_COMMAND_TYPE_CUBE: 
            buffer = (buffer_t){
                .raw_data = (u8 *)DEFAULT_CUBE_VERTICES_WITH_NORMALS_24,
                .size = sizeof(DEFAULT_CUBE_VERTICES_WITH_NORMALS_24)
            };
        break;
        case RENDER_COMMAND_TYPE_CAPSULE:
            buffer = (buffer_t){
                .raw_data = (u8 *)DEFAULT_CAPSULE_VERTICES_WITH_NORMALS,
                .size = sizeof(DEFAULT_CAPSULE_VERTICES_WITH_NORMALS)
            };
        break;
        case RENDER_COMMAND_TYPE_CUSTOM: 
            buffer = __render_command_merge_all_vtx_together(commands, arena);
        break;
        default: eprint("unknown render type");
    }

    return buffer;
}

buffer_t __render_command_merge_all_vtx_together(const list_t * const commands, arena_t * const arena)
{
    u8 maximum_size = 0;
    list_iterator(commands, iter) 
    {
        const render_command_t *command = iter;
        maximum_size += command->call_config.vtx[VBO_STREAM_TYPE_GEOMETRY].size;
    }

    u8 *buffer = arena_reserve_raw(arena, maximum_size);
    u8 top = 0;
    list_iterator(commands, iter) 
    {
        const render_command_t *command = iter;
        memcpy(buffer + top, command->call_config.vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data, command->call_config.vtx[VBO_STREAM_TYPE_GEOMETRY].size);
        top += command->call_config.vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data;
    }
    return (buffer_t) {
        .raw_data = buffer,
        .size = maximum_size
    };
}

#endif
