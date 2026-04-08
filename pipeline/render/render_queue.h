#pragma once
#include <poglib/gfx/glrenderer3d.h>
#include "poglib/basic/arena.h"
#include "poglib/basic/common.h"
#include "poglib/basic/dbg.h"
#include "poglib/basic/ds/list.h"
#include "poglib/gfx/gl/renderconfig.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/gfx/gl/vbo_stream_types.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/pipeline/render/render_command.h"

renderqueue_t       renderqueue_init(void);
void                renderqueue_pass_command(renderqueue_t * const self, const rendercommand_t command);
void                renderqueue_dispatch(renderqueue_t * const self);
void                renderqueue_destroy(renderqueue_t *self);

#ifndef IGNORE_RENDER_QUEUE_IMPLEMENTATION

void renderqueue__internal_validate_command(rendercommand_t);
bool renderqueue__internal_check_for_batchable_commands(renderqueue_t * const queue, rendercommand_t command);
void renderqueue__internal_flush(renderqueue_t *const self);
bool renderqueue__internal_is_instanced_only(rendercommand_types type);

renderqueue_t renderqueue_init(void)
{
    arena_t arena = arena_init(NULL, 3 * MB);
    return (renderqueue_t) {
        .bucket_ready_count = 0,
        .buckets = {0},
        .arena = arena, 
        .internal = {
            .instance_shader = glshader__file_init(
                str(POGLIB_ROOT_DIR"/pipeline/render/shader/instance-vtx.glsl"),
                str(POGLIB_ROOT_DIR"/pipeline/render/shader/instance-frag.glsl"),
                &arena
            ),
        }
    };
}

void renderqueue_pass_command(renderqueue_t *const self, const rendercommand_t command)
{
    ASSERT(self);

    renderqueue__internal_validate_command(command);

    if (renderqueue__internal_check_for_batchable_commands(self, command)) {
        return;
    }

    const bool is_bucket_ready = self->buckets[self->bucket_ready_count].is_ready;
    if (!is_bucket_ready) {
        self->buckets[self->bucket_ready_count].render_commands = list_init(rendercommand_t);
        self->buckets[self->bucket_ready_count].is_ready = true;
        self->buckets[self->bucket_ready_count].type = command.type;
        list_append(&self->buckets[self->bucket_ready_count].render_commands, command);
        self->bucket_ready_count++;
        return;
    }

    list_append(&self->buckets[self->bucket_ready_count].render_commands, command);
}

void renderqueue_destroy(renderqueue_t *self)
{
    for (u8 idx = 0; idx < MAX_RENDER_BUCKETS_ALLOWED; idx++)
    {
        if (!self->buckets[idx].is_ready) continue;
        list_destroy(&self->buckets[idx].render_commands);
    }
    glshader_destroy(&self->internal.instance_shader);
    arena_destroy(&self->arena);
}

void renderqueue__internal_validate_command(rendercommand_t command)
{
    switch(command.type)
    {
        case RENDER_COMMAND_TYPE_CAPSULE:
        case RENDER_COMMAND_TYPE_CUBE:
            //NOTE: this geometrys are instanced - currently only transforms are passed to it
        break;
        case RENDER_COMMAND_TYPE_CUSTOM: 
            ASSERT(command.call_config.vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data == NULL);
        break;
        case RENDER_COMMAND_TYPE_CUSTOM_WITH_INSTANCING: 
            if(command.call_config.vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data)
                eprint("Instancing uses a common geometry, avoid initializing geometry data");
            if(!command.call_config.vtx[VBO_STREAM_TYPE_INSTANCE].raw_data) 
                eprint("Custom render types are configured to be instanced, expecting instance buffer but found uninitialized!");
        break;
        default: eprint("unknown type");
    }
}

void renderqueue__internal_add_to_batch(list_t * const render_commands, const rendercommand_t command)
{
    list_append(render_commands, command);
}

bool renderqueue__internal_check_for_batchable_commands(renderqueue_t *const queue, rendercommand_t command)
{
    const bool is_instanced_only = renderqueue__internal_is_instanced_only(command.type);

    for (u8 idx = 0; idx < queue->bucket_ready_count; idx++)
    {
        list_t *const commands = &queue->buckets[idx].render_commands;
        if (!queue->buckets[idx].is_ready)  continue;

        if (command.type == queue->buckets[idx].type) {
            renderqueue__internal_add_to_batch(commands, command);
            return true;
        }

        if (is_instanced_only) 
            continue;

        //FIXME: this will fail for custom meshes!!
        const rendercommand_t * const existing_render_command_in_bucket = list_get_value(commands, 0);

        const bool has_same_texture = command.call_config.textures.count 
            && existing_render_command_in_bucket->call_config.textures.count 
            && rendercommand_are_all_textures_the_same(&command, existing_render_command_in_bucket);

        const bool has_same_shader = command.call_config.shader_config.shader 
            && existing_render_command_in_bucket->call_config.shader_config.shader
            && command.call_config.shader_config.shader->id == existing_render_command_in_bucket->call_config.shader_config.shader->id;

        const bool has_same_attributes = rendercommand_are_all_attrs_the_same(existing_render_command_in_bucket, &command);

        if (has_same_shader && has_same_texture && has_same_attributes) {
            renderqueue__internal_add_to_batch(commands, command);
            return true;
        }
    }
    return false;
}

void renderqueue_dispatch(renderqueue_t *const self)
{
    if (!self->bucket_ready_count) return;
    ASSERT(self->bucket_ready_count < MAX_DRAW_CALLS_PER_FRAME_COUNT);

    u8 total_render_command = 0;
    glrendercall_t calls[MAX_DRAW_CALLS_PER_FRAME_COUNT] = {0};

    for (u8 idx = 0; idx < self->bucket_ready_count; idx++)
    {
        const list_t *bucket_commands = &self->buckets[idx].render_commands;
        if (!bucket_commands->len) continue;

        const rendercommand_t *command = list_get_value(bucket_commands, 0);

        const buffer_t vtx_buffer               = rendercommand_get_vtx_buffer(bucket_commands, &self->arena);
        const buffer_t instance_buffer          = rendercommand_get_instance_buffer(bucket_commands, &self->arena);
        const buffer_t idx_buffer               = rendercommand_get_idx_buffer(bucket_commands, &self->arena);
        const glvtx_attributelist_t attr_list   = rendercommand_get_attrs(bucket_commands);
        const glshaderconfig_t shader_config    = rendercommand_get_shaderconfig(bucket_commands, self);
        const bool enable_instancing            = renderqueue__internal_is_instanced_only(command->type);

        const u32 instancing_count = enable_instancing ? bucket_commands->len : 0;
        calls[total_render_command] = (glrendercall_t ) {
            .draw_mode = command->draw_mode,
            .allow_empty_vtx_buffer = false,
            .is_wireframe = false,
            .vtx = {
                [VBO_STREAM_TYPE_GEOMETRY] = vtx_buffer,
                [VBO_STREAM_TYPE_INSTANCE] = instance_buffer
            },
            .attrs = attr_list,
            .idx = {
                .data = idx_buffer.raw_data,
                .nmemb = idx_buffer.size / sizeof(u32),
            },
            .shader_config = shader_config,
            .textures = !enable_instancing ? command->call_config.textures : (gltexturelist_t){0},
            .instancing = {
                .enable = enable_instancing,
                .count = instancing_count
            }
        };

        glrenderer3d_drawcall(calls[total_render_command]);
        total_render_command++;
    }
    renderqueue__internal_flush(self);
}

void renderqueue__internal_flush(renderqueue_t * const self)
{
    for (u8 idx = 0; idx < self->bucket_ready_count; idx++)
    {
        list_clear(&self->buckets[idx].render_commands);
    }
    arena_clear(&self->arena);
}

bool renderqueue__internal_is_instanced_only(rendercommand_types type)
{
    return type & (RENDER_COMMAND_TYPE_CUBE | RENDER_COMMAND_TYPE_CAPSULE);
}

//FIXME: 
//1. O(n**2) problem with `render_queue_pass_command`, fix - State Sorting 

//TODO:
//1. Have texture support for instancing
#endif
