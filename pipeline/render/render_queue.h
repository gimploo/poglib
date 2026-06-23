#pragma once
#include <poglib/gfx/glrenderer3d.h>
#include "poglib/basic/common.h"
#include "poglib/basic/dbg.h"
#include "poglib/basic/ds/list.h"
#include "poglib/gfx/gl/instance-buffer.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/pipeline/render/render_command.h"
#include "poglib/util/asset.h"

renderqueue_t       renderqueue_init(arena_t *arena);
void                renderqueue_pass_command(renderqueue_t * const self, const rendercommand_t command);
void                renderqueue_dispatch(renderqueue_t * const self);
void                renderqueue_flush(renderqueue_t * const self);
void                renderqueue_destroy(renderqueue_t * const self);

#ifndef IGNORE_RENDER_QUEUE_IMPLEMENTATION

void renderqueue__internal_validate_command(rendercommand_t);
bool renderqueue__internal_check_for_batchable_commands(renderqueue_t * const queue, rendercommand_t command);
void renderqueue__internal_add_to_bucket(list_t * const render_commands, const rendercommand_t command);
void rendercommand__internal_shader_upload_uniforms(const rendercommand_t * const command);

renderqueue_t renderqueue_init(arena_t *arena)
{
    return (renderqueue_t) {
        .buckets = {0},
        .arena = arena_init(arena, 2 * MB),
        .internal = {
            .instancebuffer = glinstancebuffer_init(2 * MB),
        }
    };
}

void renderqueue__internal_bucket_init_and_add_command(renderqueue_t *const self, const rendercommand_t command, const u16 idx)
{
    self->buckets[idx] = list_init(rendercommand_t, &self->arena);
    list_append(&self->buckets[idx], command);
}

void renderqueue_pass_command(renderqueue_t *const self, rendercommand_t command)
{
    ASSERT(self);
    ASSERT(command.instance.size <= sizeof(command.instance.raw_data));

    renderqueue__internal_validate_command(command);

    if (renderqueue__internal_check_for_batchable_commands(self, command)) {
        return;
    }

    for (u8 idx = 0; idx < ARRAY_LEN(self->buckets); idx++)
    {
        if (!list_is_init(&self->buckets[idx])) {
            renderqueue__internal_bucket_init_and_add_command(self, command, idx);
            return;
        }

        if (self->buckets[idx].len == 0) {
            renderqueue__internal_add_to_bucket(&self->buckets[idx], command);
            return;
        }
    }

    eprint("Buckets fully filled");
}

void renderqueue_destroy(renderqueue_t * const self)
{
    for (u8 idx = 0; idx < MAX_RENDER_BUCKETS_ALLOWED; idx++)
    {
        if (!list_is_init(&self->buckets[idx])) 
            continue;

        list_destroy(&self->buckets[idx]);
    }
    glinstancebuffer_destroy(&self->internal.instancebuffer);
    arena_destroy(&self->arena);
}

void renderqueue__internal_validate_command(const rendercommand_t command)
{
    //TODO: validation for textures also (solution - sort the command texture ids in desc order before compare to avoid
    //unintentional mismatches - or through an error during runtime to reorder the textures (better) so we can avoid the sorting
    //all together

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
    for (u8 idx = 0; idx < MAX_RENDER_BUCKETS_ALLOWED; idx++)
    {
        list_t *const commands = &queue->buckets[idx];

        if (!list_is_init(commands)) continue;

        const rendercommand_t * const first_render_command = commands->len
            ? list_get_value(commands, 0)
            : NULL;

        if (!first_render_command)                                                  continue;
        if (command.mesh->vao_id != first_render_command->mesh->vao_id)             continue;
        if (command.draw_mode != first_render_command->draw_mode)                   continue;
        if (command.enable_wireframe != first_render_command->enable_wireframe)     continue;

        const bool has_same_shader = rendercommand__internal_compare_shader_and_uniforms(
            first_render_command, 
            &command
        );
        if (!has_same_shader) continue;

        const bool has_same_texture = rendercommand__internal_compare_textures_ids(
            first_render_command, 
            &command
        );
        if (!has_same_texture) continue;

        const bool has_same_instance_size = first_render_command->instance.size == command.instance.size;
        if (!has_same_instance_size) 
            continue;

        renderqueue__internal_add_to_bucket(commands, command);
        return true;
    }
    return false;
}

i32 renderqueue__internal_qsort_compare(const void *x, const void *y)
{
    const list_t *bucket_a = (const list_t *)x;
    const list_t *bucket_b = (const list_t *)y;

    bool has_a = bucket_a->len > 0;
    bool has_b = bucket_b->len > 0;

    if (!has_a && !has_b) return 0;
    if (!has_a) return 1;
    if (!has_b) return -1;

    const rendercommand_t *cmd_a = list_get_value(bucket_a, 0);
    const rendercommand_t *cmd_b = list_get_value(bucket_b, 0);

    const u32 shaderid_a = (cmd_a->material.shader.data == NULL) 
        ? 0
        : cmd_a->material.shader.data->id;

    const u32 shaderid_b = (cmd_b->material.shader.data == NULL) 
        ? 0
        : cmd_b->material.shader.data->id;

    if (shaderid_a < shaderid_b) return 1;
    if (shaderid_a > shaderid_b) return -1;
    return 0;
}

void renderqueue__internal_render_all_meshes_in_bucket(const list_t * const bucket)
{
    list_iterator(bucket, iter)
    {
        const rendercommand_t *const command = iter;
        const glshader_t * const shader = command->material.shader.data;
        ASSERT(shader);

        GL_CHECK(glBindVertexArray(command->mesh->vao_id));

        glshader_upload_uniforms(shader, command->material.shader.uniforms);

        GL_CHECK(glDrawElements(
            command->draw_mode, 
            command->mesh->index_count, 
            GL_UNSIGNED_INT, 
            0
        ));
    }
}

void renderqueue_dispatch(renderqueue_t *const self)
{
    qsort(self->buckets, ARRAY_LEN(self->buckets), sizeof(list_t), renderqueue__internal_qsort_compare);

    u32 binded_shader_id = 0;
    u32 instance_starting_offset = 0;

    for (u8 idx = 0; idx < MAX_DRAW_CALLS_PER_FRAME_COUNT; idx++)
    {
        const list_t *bucket_commands = &self->buckets[idx];
        if (!bucket_commands->len)          continue;

        const rendercommand_t *first_command = list_get_value(bucket_commands, 0);
        ASSERT(first_command->mesh->vao_id);

        if (first_command->enable_wireframe)    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        else                                    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

        //NOTE: bind shader
        const glshader_t * const shader = first_command->material.shader.data;
        if (binded_shader_id != shader->id) {
            GL_CHECK(glUseProgram(shader->id));
            binded_shader_id = shader->id;
        }

        //NOTE: bind textures
        for(u8 tex_idx = 0; tex_idx < first_command->material.textures.count; tex_idx++) {
            GL_CHECK(glActiveTexture(GL_TEXTURE0 + tex_idx));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, first_command->material.textures.ids[tex_idx]));
        }

        //NOTE: Use instance if configured
        const bool is_instanced = first_command->instance.size > 0;
        if (is_instanced) {
            //NOTE: copy over instace data into the gl instance buffer
            instance_starting_offset = glinstancebuffer_get_current_offest(&self->internal.instancebuffer);
            list_iterator(bucket_commands, iter)
            {
                const rendercommand_t * const cmd = (rendercommand_t *)iter;
                glinstancebuffer_push(
                    &self->internal.instancebuffer,
                    cmd->instance.raw_data,
                    cmd->instance.size
                );
            }

            GL_CHECK(glBindVertexArray(first_command->mesh->vao_id));

            glinstancebuffer_bind(
                &self->internal.instancebuffer, 
                instance_starting_offset,
                bucket_commands->len * first_command->instance.size);

            if (binded_shader_id)
                glshader_upload_uniforms(shader, first_command->material.shader.uniforms);

            GL_CHECK(glDrawElementsInstanced(
                first_command->draw_mode, 
                first_command->mesh->index_count, 
                GL_UNSIGNED_INT, 
                0,
                bucket_commands->len));

        } else {

            renderqueue__internal_render_all_meshes_in_bucket(&self->buckets[idx]);

        }
        GL_CHECK(glBindVertexArray(0));
    }
    glinstancebuffer_unbind(&self->internal.instancebuffer);
}

void renderqueue_flush(renderqueue_t *const self)
{
    for (u8 idx = 0; idx < MAX_RENDER_BUCKETS_ALLOWED; idx++)
    {
        if (list_is_init(&self->buckets[idx]))
            list_clear(&self->buckets[idx]);
    }
}

#endif
