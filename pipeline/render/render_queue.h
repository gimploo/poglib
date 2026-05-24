#pragma once
#include <poglib/gfx/glrenderer3d.h>
#include "poglib/basic/arena.h"
#include "poglib/basic/common.h"
#include "poglib/basic/dbg.h"
#include "poglib/basic/ds/list.h"
#include "poglib/gfx/gl/instance-buffer.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/pipeline/render/render_command.h"
#include "poglib/util/asset.h"

renderqueue_t       renderqueue_init(void);
void                renderqueue_pass_command(renderqueue_t * const self, const rendercommand_t command);
void                renderqueue_dispatch(renderqueue_t * const self);
void                renderqueue_destroy(renderqueue_t *self);

#ifndef IGNORE_RENDER_QUEUE_IMPLEMENTATION

void renderqueue__internal_validate_command(rendercommand_t);
bool renderqueue__internal_check_for_batchable_commands(renderqueue_t * const queue, rendercommand_t command);
void renderqueue__internal_flush(renderqueue_t *const self);
void renderqueue__internal_add_to_bucket(list_t * const render_commands, const rendercommand_t command);
void rendercommand__internal_shader_upload_uniforms(const rendercommand_t * const command);

renderqueue_t renderqueue_init(void)
{
    arena_t arena = arena_init(NULL, 3 * MB);
    return (renderqueue_t) {
        .bucket_ready_count = 0,
        .buckets = {0},
        .internal = {
            .arena = arena, 
            .instancebuffer = glinstancebuffer_init(2 * MB),
        }
    };
}

void renderqueue__internal_bucket_init_and_add_command(renderqueue_t *const self, const rendercommand_t command)
{
    self->buckets[self->bucket_ready_count] = (renderqueue__internal_bucket_type){
        .render_commands = list_init(rendercommand_t),
        .draw_mode = command.draw_mode,
        .is_instanced = command.instance.size > 0,
        .enable_wireframe = command.enable_wireframe
    };
    list_append(&self->buckets[self->bucket_ready_count].render_commands, command);
    self->bucket_ready_count++;
}

void renderqueue_pass_command(renderqueue_t *const self, rendercommand_t command)
{
    ASSERT(self);

    renderqueue__internal_validate_command(command);

    if (renderqueue__internal_check_for_batchable_commands(self, command)) {
        return;
    }

    if (!list_is_init(&self->buckets[self->bucket_ready_count].render_commands)) {
        renderqueue__internal_bucket_init_and_add_command(self, command);
        return;
    }

    renderqueue__internal_add_to_bucket(&self->buckets[self->bucket_ready_count].render_commands, command);
}

void renderqueue_destroy(renderqueue_t *self)
{
    for (u8 idx = 0; idx < MAX_RENDER_BUCKETS_ALLOWED; idx++)
    {
        if (!list_is_init(&self->buckets[idx].render_commands)) 
            continue;

        list_destroy(&self->buckets[idx].render_commands);
    }
    arena_destroy(&self->internal.arena);
    glinstancebuffer_destroy(&self->internal.instancebuffer);
}

void renderqueue__internal_validate_command(const rendercommand_t command)
{
    {   //NOTE: Mesh validation
        ASSERT(command.mesh);
        ASSERT(command.mesh->vao_id > 0);
        ASSERT(command.mesh->attribute_count > 0);
        ASSERT(command.mesh->index_count > 0);
    }

    {
        //NOTE: Shader + uniform validation
        ASSERT(command.material.shader.data);
        if (command.material.shader.data && (command.material.shader.uniforms.count != glshader_get_uniform_count(command.material.shader.data)))
            eprint("render command is missing uniform value, check shader `%.*s` or `%.*s` to find missing uniforms", 
                command.material.shader.data->vs.len, command.material.shader.data->vs.data, 
                command.material.shader.data->fg.len, command.material.shader.data->fg.data);
    }
}

void renderqueue__internal_add_to_bucket(list_t * const render_commands, const rendercommand_t command)
{
    list_append(render_commands, command);
}

bool renderqueue__internal_check_for_batchable_commands(renderqueue_t *const queue, rendercommand_t command)
{
    for (u8 idx = 0; idx < queue->bucket_ready_count; idx++)
    {
        list_t *const commands = &queue->buckets[idx].render_commands;

        if (!list_is_init(&queue->buckets[idx].render_commands))                continue;
        if (command.draw_mode != queue->buckets[idx].draw_mode)                 continue;
        if (command.enable_wireframe != queue->buckets[idx].enable_wireframe)   continue;

        const rendercommand_t * const existing_render_command_in_bucket = commands->len
            ? list_get_value(commands, 0)
            : NULL;

        if (existing_render_command_in_bucket) {
            const bool has_same_shader = rendercommand__internal_compare_shader_and_uniforms(
                existing_render_command_in_bucket, 
                &command
            );
            if (!has_same_shader) continue;

            const bool has_same_texture = rendercommand__internal_compare_textures_ids(
                existing_render_command_in_bucket, 
                &command
            );
            if (!has_same_texture) continue;
        }

        renderqueue__internal_add_to_bucket(commands, command);
        return true;
    }
    return false;
}

i32 renderqueue__internal_qsort_compare(const void *x, const void *y)
{
    const renderqueue__internal_bucket_type *bucket_a = (const renderqueue__internal_bucket_type *)x;
    const renderqueue__internal_bucket_type *bucket_b = (const renderqueue__internal_bucket_type *)y;

    bool has_a = bucket_a->render_commands.len > 0;
    bool has_b = bucket_b->render_commands.len > 0;
    if (!has_a && !has_b) return 0;
    if (!has_a) return 1;
    if (!has_b) return -1;

    const rendercommand_t *cmd_a = list_get_value(&bucket_a->render_commands, 0);
    const rendercommand_t *cmd_b = list_get_value(&bucket_b->render_commands, 0);

    const u32 shaderid_a = (cmd_a->material.shader.data == NULL) 
        ? 0
        : cmd_a->material.shader.data->id;

    const u32 shaderid_b = (cmd_b->material.shader.data == NULL) 
        ? 0
        : cmd_b->material.shader.data->id;

    if (shaderid_a < shaderid_b) return -1;
    if (shaderid_a > shaderid_b) return 1;
    return 0;
}


void renderqueue_dispatch(renderqueue_t *const self)
{
    if (!self->bucket_ready_count) return;
    ASSERT(self->bucket_ready_count < MAX_DRAW_CALLS_PER_FRAME_COUNT);

    qsort(self->buckets, ARRAY_LEN(self->buckets), sizeof(renderqueue__internal_bucket_type), renderqueue__internal_qsort_compare);

    u32 current_binded_shader_id = 0;

    for (u8 idx = 0; idx < self->bucket_ready_count; idx++)
    {
        const list_t *bucket_commands = &self->buckets[idx].render_commands;
        if (!bucket_commands->len) continue;

        const rendercommand_t *first_command = list_get_value(bucket_commands, 0);
        const bool is_instanced = first_command->instance.size > 0;

        if (is_instanced) {
            list_iterator(bucket_commands, iter)
            {
                rendercommand_t *cmd = (rendercommand_t *)iter;
                glinstancebuffer_push(
                    &self->internal.instancebuffer,
                    cmd->instance.raw_data,
                    cmd->instance.size
                );
            }
        }
        const glshader_t * const shader = first_command->material.shader.data;
        if (current_binded_shader_id != shader->id) {
            GL_CHECK(glUseProgram(shader->id));
            glshader_upload_uniforms(shader, first_command->material.shader.uniforms);
            current_binded_shader_id = shader->id;
        }

        if (first_command->material.textures.count) {
            for(u8 tex_idx = 0; tex_idx < first_command->material.textures.count; tex_idx++) {
                GL_CHECK(glActiveTexture(GL_TEXTURE0 + tex_idx));
                GL_CHECK(glBindTexture(GL_TEXTURE_2D, first_command->material.textures.ids[tex_idx]));
            }
        }

        ASSERT(first_command->mesh->vao_id);
        GL_CHECK(glBindVertexArray(first_command->mesh->vao_id));

        if (is_instanced) {
            glinstancebuffer_bind(
                &self->internal.instancebuffer, 
                bucket_commands->len * first_command->instance.size);
        }

        if (first_command->enable_wireframe) {
            GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        } else {
            GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        if (is_instanced) {
            GL_CHECK(glDrawElementsInstanced(
                first_command->draw_mode, 
                first_command->mesh->index_count, 
                GL_UNSIGNED_INT, 
                0,
                bucket_commands->len));
        } else {
            GL_CHECK(glDrawElements(
                first_command->draw_mode, 
                first_command->mesh->index_count, 
                GL_UNSIGNED_INT, 
                0
            ));
        }

        if (is_instanced) {
            glinstancebuffer_unbind(&self->internal.instancebuffer);
        }
        GL_CHECK(glBindVertexArray(0));
    }
    renderqueue__internal_flush(self);
}

void renderqueue__internal_flush(renderqueue_t * const self)
{
    for (u8 idx = 0; idx < self->bucket_ready_count; idx++)
    {
        list_clear(&self->buckets[idx].render_commands);
    }
    arena_clear(&self->internal.arena);
}


//FIXME: 
//O(n**2) problem with `render_queue_pass_command`, fix - State Sorting 

#endif
