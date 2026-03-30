#pragma once
#include <poglib/gfx/glrenderer3d.h>
#include "poglib/basic/arena.h"
#include "poglib/basic/dbg.h"
#include "poglib/basic/ds/list.h"
#include "poglib/gfx/gl/vbo_stream_types.h"
#include "poglib/pipeline/render/render_command.h"

//FIXME: 
//1. VTX is not being generated for common types
//2. VTX buffers are not merged during draw call (use arenas)
//3. O(n**2) problem with `render_queue_pass_command`

#define MAX_RENDER_BUCKETS_ALLOWED 255

//NOTE: this is bucket will sort the commands in order always
typedef struct {
    u8 len;
    struct {
        bool is_ready; //to know whether the list is initialized
        list_t render_commands;
    } buckets[MAX_RENDER_BUCKETS_ALLOWED];
    arena_t arena;
} render_queue_t;

render_queue_t      render_queue_init(void);
void                render_queue_pass_command(render_queue_t * const self, const render_command_t command);
void                render_queue_dispatch(render_queue_t * const self);
void                render_queue_destroy(render_queue_t *self);

#ifndef IGNORE_RENDER_QUEUE_IMPLEMENTATION

void __render_queue_validate_command(render_command_t);
bool __render_queue_check_for_batchable_commands(render_queue_t * const queue, render_command_t command);
void __render_queue_flush(render_queue_t *const self);

render_queue_t render_queue_init(void)
{
    return (render_queue_t) {
        .len = 0,
        .buckets = {0},
        .arena = arena_init(NULL, 3 * MB)
    };
}

void render_queue_pass_command(render_queue_t *const self, const render_command_t command)
{
    ASSERT(self);

    __render_queue_validate_command(command);
    if (__render_queue_check_for_batchable_commands(self, command)) {
        return;
    }

    const bool is_bucket_ready = self->buckets[self->len].is_ready;
    if (!is_bucket_ready) {
        self->buckets[self->len].render_commands = list_init(render_command_t);
        self->buckets[self->len].is_ready = true;
    }

    list_append(&self->buckets[self->len].render_commands, command);
    self->len++;
}

void render_queue_destroy(render_queue_t *self)
{
    for (u8 idx = 0; idx < MAX_RENDER_BUCKETS_ALLOWED; idx++)
    {
        if (!self->buckets[idx].is_ready) continue;
        list_destroy(&self->buckets[idx].render_commands);
    }
    arena_destroy(&self->arena);
}

void __render_queue_validate_command(render_command_t command)
{
    switch(command.type)
    {
        case RENDER_COMMAND_TYPE_CUBE:
            ASSERT(command.handles.vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data == NULL);
            ASSERT(command.handles.idx.data == NULL);
            if(!command.handles.vtx[VBO_STREAM_TYPE_INSTANCE].raw_data) 
                eprint("Cube render types are instanced always, expecting instance buffer but found uninitialized!");
        break;
        case RENDER_COMMAND_TYPE_CUSTOM: 
            ASSERT(command.handles.vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data == NULL);
        break;
        case RENDER_COMMAND_TYPE_CUSTOM_WITH_INSTANCING: 
            if(command.handles.vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data)
                eprint("Instancing uses a common geometry, avoid initializing geometry data");
            if(!command.handles.vtx[VBO_STREAM_TYPE_INSTANCE].raw_data) 
                eprint("Custom render types are configured to be instanced, expecting instance buffer but found uninitialized!");
        break;
        default: eprint("unknown type");
    }
}

void __render_queue_add_to_batch(list_t * const render_commands, const render_command_t command)
{
    list_append(render_commands, command);
}

bool __render_queue_check_for_batchable_commands(render_queue_t *const queue, render_command_t command)
{
    for (u8 idx = 0; idx < queue->len; idx++)
    {
        list_t *const commands = &queue->buckets[idx].render_commands;
        if (!queue->buckets[idx].is_ready)  continue;
        if (!commands->len)                 continue;

        const render_command_t *render_command = list_get_value(commands, 0);
        if (command.type == render_command->type) {
            __render_queue_add_to_batch(commands, command);
            return true;
        }

        const bool has_same_texture = command.handles.textures.count 
            && render_command->handles.textures.count 
            && render_command_are_all_textures_the_same(&command, render_command);

        const bool has_same_shader = command.handles.shader_config.shader 
            && render_command->handles.shader_config.shader
            && command.handles.shader_config.shader->id == render_command->handles.shader_config.shader->id;

        const bool has_same_attributes = render_command_are_all_attrs_the_same(render_command, &command);

        if (has_same_shader && has_same_texture && has_same_attributes) {
            __render_queue_add_to_batch(commands, command);
            return true;
        }
    }
    return false;
}

void render_queue_dispatch(render_queue_t *const self)
{
    if (!self->len) return;
    ASSERT(self->len < MAX_DRAW_CALLS_PER_FRAME_COUNT);

    u8 total_render_command = 0;
    glrendercall_t calls[MAX_DRAW_CALLS_PER_FRAME_COUNT] = {0};

    for (u8 idx = 0; idx < self->len; idx++)
    {
        const list_t *command_list = &self->buckets[idx].render_commands;
        if (!command_list->len) continue;

        const render_command_t *command = list_get_value(command_list, 0);

        const buffer_t vtx_buffer = render_command_get_vtx_buffer(command_list, &self->arena);
        const buffer_t idx_buffer = render_command_get_idx_buffer(command_list, &self->arena);

        const bool enable_instancing = command->type & (RENDER_COMMAND_TYPE_CUSTOM_WITH_INSTANCING | RENDER_COMMAND_TYPE_CUBE);
        const u32 instancing_count = enable_instancing ? command_list->len : 1;
        calls[total_render_command] = (glrendercall_t ) {
            .draw_mode = command->draw_mode,
            .allow_empty_vtx_buffer = false,
            .is_wireframe = false,
            .vtx = command->handles.vtx,
            .attrs = command->handles.attrs.data,
            .idx = {
                .nmemb = command->handles.idx.nmemb,
                .data = command->handles.idx.data
            },
            .shader_config = command->handles.shader_config,
            .textures = command->handles.textures,
            .instancing = {
                .enable = enable_instancing,
                .count = instancing_count
            }
        };
        total_render_command++;
    }
    glrenderer3d_draw((glrendererconfig_t) {
        .calls = {
            .count = total_render_command,
            .call = calls
        }
    });

    __render_queue_flush(self);
}

void __render_queue_flush(render_queue_t * const self)
{
    for (u8 idx = 0; idx < self->len; idx++)
    {
        list_clear(&self->buckets[idx].render_commands);
    }
}

#endif
