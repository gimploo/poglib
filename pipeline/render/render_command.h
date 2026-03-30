#pragma once
#include "poglib/basic/arena.h"
#include "poglib/basic/ds/list.h"
#include "poglib/gfx/gl/renderconfig.h"
#include "poglib/gfx/gl/types.h"
#include "poglib/gfx/gl/vtx_attribute.h"
#include <poglib/gfx/glrenderer3d.h>

typedef enum {
    RENDER_COMMAND_TYPE_CUSTOM = 0,
    RENDER_COMMAND_TYPE_CUBE = 1,
    RENDER_COMMAND_TYPE_CUSTOM_WITH_INSTANCING = 3,
    RENDER_COMMAND_TYPE_COUNT,
} render_command_types;

typedef enum {
    RENDER_COMMAND_DRAW_MODE_TRIANGLE = GL_TRIANGLES,
    RENDER_COMMAND_DRAW_MODE_LINES = GL_LINES,
    RENDER_COMMAND_DRAW_MODE_COUNT,
} render_command_draw_mode;

typedef struct {
    render_command_types        type;
    render_command_draw_mode    draw_mode;
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
    } handles;

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
    ASSERT(render_command->handles.attrs.len <= GL_VTX_ATTRIBUTE_TYPE_COUNT);
    ASSERT(command->handles.attrs.len <= GL_VTX_ATTRIBUTE_TYPE_COUNT);

    if (command->handles.attrs.len != render_command->handles.attrs.len)
        return false;

    for (gl_vtx_attribute_type idx = 0; idx < command->handles.attrs.len; idx++)
    {
        const glvtx_attribute_t *attr_x = &command->handles.attrs.data[idx];
        const glvtx_attribute_t *attr_y = &render_command->handles.attrs.data[idx];

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

    if (command->handles.textures.count != render_command->handles.textures.count)
        return false;

    for (u8 idx = 0; idx < command->handles.textures.count; idx++)
    {
        if (command->handles.textures.items[idx].source.normal_texture->id != render_command->handles.textures.items[idx].source.normal_texture->id)
            return false;
    }
    return true;
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
                .raw_data = (u8 *)&DEFAULT_CUBE_INDICES_8,
                .size = sizeof(DEFAULT_CUBE_INDICES_8)
            };
        break;
        case RENDER_COMMAND_TYPE_CUSTOM: 
            eprint("TODO: for custom idx");
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
                .raw_data = (u8 *)&DEFAULT_CUBE_VERTICES_8,
                .size = sizeof(DEFAULT_CUBE_VERTICES_8)
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
        maximum_size += command->handles.vtx[VBO_STREAM_TYPE_GEOMETRY].size;
    }

    u8 *buffer = arena_reserve_raw(arena, maximum_size);
    u8 top = 0;
    list_iterator(commands, iter) 
    {
        const render_command_t *command = iter;
        memcpy(buffer + top, command->handles.vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data, command->handles.vtx[VBO_STREAM_TYPE_GEOMETRY].size);
        top += command->handles.vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data;
    }
    return (buffer_t) {
        .raw_data = buffer,
        .size = maximum_size
    };
}

#endif
