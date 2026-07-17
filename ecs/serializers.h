#pragma once
#include "poglib/basic/str.h"
#include <poglib/basic.h>
#include <poglib/ecs/component/types.h>

#define SERIALIZE_KV(file, buf, indent, fmt, ...) do { \
    snprintf(buf, sizeof(buf), "%s" fmt "\n", indent, ##__VA_ARGS__); \
    file_writeline(file, buf); \
} while(0)

/* ======================================================================
 *  TRANSFORM
 * ==================================================================== */

INTERNAL void ecs_transform_serializer(file_t *const file, const void *const cmp_data)
{
    const ecs_component_transform_t *t = cmp_data;
    char buf[WORD] = {0};

    SERIALIZE_KV(file, buf, "\t", "%i transform 5", ECS_CMP_TRANSFORM_IDX);
    SERIALIZE_KV(file, buf, "\t\t", "position %f %f %f", t->position.x, t->position.y, t->position.z);
    SERIALIZE_KV(file, buf, "\t\t", "orientation %f %f %f %f", t->orientation.x, t->orientation.y, t->orientation.z, t->orientation.w);
    SERIALIZE_KV(file, buf, "\t\t", "scale %f %f %f", t->scale.x, t->scale.y, t->scale.z);
    SERIALIZE_KV(file, buf, "\t\t", "velocity %f %f %f", t->velocity.x, t->velocity.y, t->velocity.z);
    SERIALIZE_KV(file, buf, "\t\t", "source %d", t->source);
}

INTERNAL u64 ecs_transform_deserializer(const str_views_t lines, const u64 line_idx, void *const cmp_data, arena_t *const arena, const hashtable_t *const assetid_remaps)
{
    ecs_component_transform_t *const t = cmp_data;

    u64 cmp_idx, member_count;
    sscanf(str_lstrip(lines.views[line_idx], '\t').data, "%"SCNu64" transform ""%"SCNu64, &cmp_idx, &member_count);

    for (u8 idx = 0; idx < member_count; idx++) 
    {
        const str_t trimmed_line    = str_lstrip(lines.views[line_idx + (idx + 1)], '\t');
        const str_pair_t pair       = str_partition(trimmed_line, ' ');

        if      (str_cmp(pair.pair[0], str("position")))       sscanf(pair.pair[1].data, "%f %f %f", &t->position.x, &t->position.y, &t->position.z);
        else if (str_cmp(pair.pair[0], str("orientation")))    sscanf(pair.pair[1].data, "%f %f %f %f", &t->orientation.x, &t->orientation.y, &t->orientation.z, &t->orientation.w);
        else if (str_cmp(pair.pair[0], str("scale")))          sscanf(pair.pair[1].data, "%f %f %f", &t->scale.x, &t->scale.y, &t->scale.z);
        else if (str_cmp(pair.pair[0], str("velocity")))       sscanf(pair.pair[1].data, "%f %f %f", &t->velocity.x, &t->velocity.y, &t->velocity.z);
        else if (str_cmp(pair.pair[0], str("source")))         sscanf(pair.pair[1].data, "%d", (int *)&t->source);
    }

    return member_count;
}

/* ======================================================================
 *  MODEL
 * ==================================================================== */

INTERNAL void ecs_model_serializer(file_t *const file, const void *const cmp_data)
{
    const ecs_component_model_t *m = cmp_data;
    u8 buf[WORD] = {0};

    SERIALIZE_KV(file, buf, "\t", "%i model 1", ECS_CMP_MODEL_IDX);
    SERIALIZE_KV(file, buf, "\t\t", "asset_id %u", m->asset_id);
}

INTERNAL u64 ecs_model_deserializer(const str_views_t lines, const u64 line_idx, void *const cmp_data, arena_t *const arena, const hashtable_t *const assetid_remaps)
{
    ecs_component_model_t *const m = cmp_data;

    u64 cmp_idx, member_count;
    sscanf(str_lstrip(lines.views[line_idx], '\t').data, "%"SCNu64" model ""%"SCNu64, &cmp_idx, &member_count);

    for (u8 idx = 0; idx < member_count; idx++) 
    {
        const str_t trimmed_line    = str_lstrip(lines.views[line_idx + (idx + 1)], '\t');
        const str_pair_t pair       = str_partition(trimmed_line, ' ');

        if (str_cmp(pair.pair[0], str("asset_id"))) sscanf(pair.pair[1].data, "%u", &m->asset_id);
    }

    m->asset_id = (u64)hashtable_get_value(assetid_remaps, (hashtable_key_t){ .u32 = m->asset_id });

    return member_count;
}

/* ======================================================================
 *  MESH
 * ==================================================================== */

INTERNAL void ecs_mesh_serializer(file_t *const file, const void *const cmp_data)
{
    const ecs_component_mesh_t *m = cmp_data;
    if (m->is_scene_instanced) return;

    u8 buf[WORD] = {0};

    SERIALIZE_KV(file, buf, "\t",   "%i mesh 2", ECS_CMP_MESH_IDX);
    SERIALIZE_KV(file, buf, "\t\t", "asset_id %u", m->asset_id);
    SERIALIZE_KV(file, buf, "\t\t", "mesh_idx %u", m->mesh_idx);
}

INTERNAL u64 ecs_mesh_deserializer(const str_views_t lines, const u64 line_idx, void *const cmp_data, arena_t *const arena, const hashtable_t *const assetid_remaps)
{
    ecs_component_mesh_t *const m = cmp_data;

    u64 cmp_idx, member_count;
    sscanf(str_lstrip(lines.views[line_idx], '\t').data, "%"SCNu64" mesh ""%"SCNu64, &cmp_idx, &member_count);

    for (u8 idx = 0; idx < member_count; idx++) 
    {
        const str_t trimmed_line    = str_lstrip(lines.views[line_idx + (idx + 1)], '\t');
        const str_pair_t pair       = str_partition(trimmed_line, ' ');

        if (str_cmp(pair.pair[0], str("asset_id")))                     sscanf(pair.pair[1].data, "%u", &m->asset_id);
        else if (str_cmp(pair.pair[0], str("mesh_idx")))                sscanf(pair.pair[1].data, "%d", (int *)&m->mesh_idx);
    }

    m->asset_id = m->asset_id > GL_MESH_PRIMITIVE_TYPE_COUNT
        ? (u64)hashtable_get_value(assetid_remaps, (hashtable_key_t){ .u32 = m->asset_id })
        : m->asset_id;

    return member_count;
}
/* ======================================================================
 *  Sprite
 * ==================================================================== */

INTERNAL void ecs_sprite_serializer(file_t *const file, const void *const cmp_data)
{
    const ecs_component_sprite_t *m = cmp_data;
    u8 buf[WORD] = {0};

    SERIALIZE_KV(file, buf, "\t",   "%i sprite 2", ECS_CMP_SPRITE_IDX);
    SERIALIZE_KV(file, buf, "\t\t", "spritesheet_asset_id %u", m->spritesheet_asset_id);
    SERIALIZE_KV(file, buf, "\t\t", "sprite_idx %u", m->sprite_idx);
}

INTERNAL u64 ecs_sprite_deserializer(const str_views_t lines, const u64 line_idx, void *const cmp_data, arena_t *const arena, const hashtable_t *const assetid_remaps)
{
    ecs_component_sprite_t *const m = cmp_data;

    u64 cmp_idx, member_count;
    sscanf(str_lstrip(lines.views[line_idx], '\t').data, "%"SCNu64" sprite ""%"SCNu64, &cmp_idx, &member_count);

    for (u8 idx = 0; idx < member_count; idx++) 
    {
        const str_t trimmed_line    = str_lstrip(lines.views[line_idx + (idx + 1)], '\t');
        const str_pair_t pair       = str_partition(trimmed_line, ' ');

        if (str_cmp(pair.pair[0], str("spritesheet_asset_id")))           sscanf(pair.pair[1].data, "%u", &m->spritesheet_asset_id);
        else if (str_cmp(pair.pair[0], str("sprite_idx")))                sscanf(pair.pair[1].data, "%d", (int *)&m->sprite_idx);
    }

    const u64 remapped_assetid = (u64)hashtable_get_value_or_null(assetid_remaps, (hashtable_key_t){ .u32 = m->spritesheet_asset_id });
    m->spritesheet_asset_id = remapped_assetid ? remapped_assetid : m->spritesheet_asset_id;

    return member_count;
}
/* ======================================================================
 *  INPUT
 * ==================================================================== */

INTERNAL void ecs_input_serializer(file_t *const file, const void *const cmp_data)
{
    const ecs_component_input_t *in = cmp_data;
    const ecs_component_input_state_t *s = &in->internal.state;
    char buf[WORD] = {0};

    SERIALIZE_KV(file, buf, "\t", "%i input 7", ECS_CMP_INPUT_IDX);
    SERIALIZE_KV(file, buf, "\t\t", "direction_source %d", in->direction_source);
    SERIALIZE_KV(file, buf, "\t\t", "state_current_orientation %f %f %f %f", s->current_orientation.x, s->current_orientation.y, s->current_orientation.z, s->current_orientation.w);
    SERIALIZE_KV(file, buf, "\t\t", "state_current_position %f %f %f", s->current_position.x, s->current_position.y, s->current_position.z);
    SERIALIZE_KV(file, buf, "\t\t", "state_velocity %f %f %f", s->velocity.x, s->velocity.y, s->velocity.z);
    SERIALIZE_KV(file, buf, "\t\t", "state_front %f %f %f", s->front.x, s->front.y, s->front.z);
    SERIALIZE_KV(file, buf, "\t\t", "state_right %f %f %f", s->right.x, s->right.y, s->right.z);
    SERIALIZE_KV(file, buf, "\t\t", "state_up %f %f %f", s->up.x, s->up.y, s->up.z);
}

INTERNAL u64 ecs_input_deserializer(const str_views_t lines, const u64 line_idx, void *const cmp_data, arena_t *const arena, const hashtable_t *const assetidremap)
{
    ecs_component_input_t *const in       = cmp_data;
    ecs_component_input_state_t *const s  = &in->internal.state;

    u64 cmp_idx, member_count;
    sscanf(str_lstrip(lines.views[line_idx], '\t').data, "%"SCNu64" input ""%"SCNu64, &cmp_idx, &member_count);

    for (u8 idx = 0; idx < member_count; idx++) 
    {
        const str_t trimmed_line    = str_lstrip(lines.views[line_idx + (idx + 1)], '\t');
        const str_pair_t pair       = str_partition(trimmed_line, ' ');

        if      (str_cmp(pair.pair[0], str("direction_source")))           sscanf(pair.pair[1].data, "%d", (int *)&in->direction_source);
        else if (str_cmp(pair.pair[0], str("state_current_orientation")))  sscanf(pair.pair[1].data, "%f %f %f %f", &s->current_orientation.x, &s->current_orientation.y, &s->current_orientation.z, &s->current_orientation.w);
        else if (str_cmp(pair.pair[0], str("state_current_position")))     sscanf(pair.pair[1].data, "%f %f %f", &s->current_position.x, &s->current_position.y, &s->current_position.z);
        else if (str_cmp(pair.pair[0], str("state_velocity")))             sscanf(pair.pair[1].data, "%f %f %f", &s->velocity.x, &s->velocity.y, &s->velocity.z);
        else if (str_cmp(pair.pair[0], str("state_front")))                sscanf(pair.pair[1].data, "%f %f %f", &s->front.x, &s->front.y, &s->front.z);
        else if (str_cmp(pair.pair[0], str("state_right")))                sscanf(pair.pair[1].data, "%f %f %f", &s->right.x, &s->right.y, &s->right.z);
        else if (str_cmp(pair.pair[0], str("state_up")))                   sscanf(pair.pair[1].data, "%f %f %f", &s->up.x, &s->up.y, &s->up.z);
    }
    return member_count;
}

/* ======================================================================
 *  MATERIAL
 * ==================================================================== */

INTERNAL void ecs_material_serializer(file_t *const file, const void *const cmp_data)
{
    const ecs_component_material_t *mat = cmp_data;
    u8 buf[WORD] = {0};
    SERIALIZE_KV(file, buf, "\t", "%i material 2", ECS_CMP_MATERIAL_IDX);
    SERIALIZE_KV(file, buf, "\t\t", "shader_asset_id:%u", mat->shader_asset_id);

    u64 offset = snprintf(buf, sizeof(buf), "\t\ttextures:(%i)[ ", mat->textures.count);
    for (u64 idx = 0; idx < mat->textures.count; idx++)
    {
        offset += snprintf(buf + offset, sizeof(buf) - offset, "%u ", mat->textures.asset_ids[idx]);
    }
    snprintf(buf + offset, sizeof(buf) - offset, "]\n");
    file_writebytes(file, buf, strlen((char *)buf));
}

INTERNAL u64 ecs_material_deserializer(const str_views_t lines, const u64 line_idx, void *const cmp_data, arena_t *const arena, const hashtable_t *const assetid_remap)
{
    ecs_component_material_t *const mat = cmp_data;

    u64 cmp_idx, member_count;
    sscanf(str_lstrip(lines.views[line_idx], '\t').data, "%"SCNu64" material ""%"SCNu64, &cmp_idx, &member_count);

    for (u8 idx = 0; idx < member_count; idx++) 
    {
        const str_t trimmed_line    = str_lstrip(lines.views[line_idx + (idx + 1)], '\t');
        const str_pair_t pair       = str_partition(trimmed_line, ':');

        if (str_cmp(pair.pair[0], str("shader_asset_id"))) {

            sscanf(pair.pair[1].data, "%u", &mat->shader_asset_id);
            const u64 remapped_asset_id = (u64)hashtable_get_value_or_null(assetid_remap, (hashtable_key_t){ .u32 = mat->shader_asset_id });
            mat->shader_asset_id = !remapped_asset_id ? mat->shader_asset_id : remapped_asset_id;

        } else if (str_cmp(pair.pair[0], str("textures"))) {

            const str_pair_t texture_array = str_partition(pair.pair[1], '[');
            sscanf(texture_array.pair[0].data, "(%i)", &mat->textures.count);
            if (mat->textures.count >= ECS_COMPONENT_MATERIAL_TEXTURE_MAX_COUNT)
                return member_count;

            const char *buffer = texture_array.pair[1].data;
            char *endptr = NULL;
            for (u32 tex_idx = 0; tex_idx < mat->textures.count; tex_idx++)
            {
                if (buffer == endptr) break; 

                mat->textures.asset_ids[tex_idx] = strtol(buffer, &endptr, 10);
                buffer = endptr;
            }
        }
    }
    return member_count + mat->textures.count;
}

/* ======================================================================
 *  CAMERA
 * ==================================================================== */

INTERNAL void ecs_camera_serializer(file_t *const file, const void *const cmp_data)
{
    const ecs_component_camera_t *c = cmp_data;
    char buf[WORD] = {0};

    SERIALIZE_KV(file, buf, "\t",   "%i camera 9", ECS_CMP_CAMERA_IDX);
    SERIALIZE_KV(file, buf, "\t\t", "position %f %f %f", c->camera.position.x, c->camera.position.y, c->camera.position.z);
    SERIALIZE_KV(file, buf, "\t\t", "euler_angle %f %f", c->camera.euler_angle.x, c->camera.euler_angle.y);
    SERIALIZE_KV(file, buf, "\t\t", "direction_front %f %f %f", c->camera.direction.front.x, c->camera.direction.front.y, c->camera.direction.front.z);
    SERIALIZE_KV(file, buf, "\t\t", "direction_up %f %f %f", c->camera.direction.up.x, c->camera.direction.up.y, c->camera.direction.up.z);
    SERIALIZE_KV(file, buf, "\t\t", "direction_right %f %f %f", c->camera.direction.right.x, c->camera.direction.right.y, c->camera.direction.right.z);
    SERIALIZE_KV(file, buf, "\t\t", "mode %d", c->mode);
    SERIALIZE_KV(file, buf, "\t\t", "follow_orbit_radius %f", c->follow.orbit_radius);
    SERIALIZE_KV(file, buf, "\t\t", "follow_center_offset %f %f %f", c->follow.center_offset.x, c->follow.center_offset.y, c->follow.center_offset.z);
    SERIALIZE_KV(file, buf, "\t\t", "follow_track_entity_id %u", c->follow.track_entity_id);
}

INTERNAL u64 ecs_camera_deserializer(const str_views_t lines, const u64 line_idx, void *const cmp_data, arena_t *const arena, const hashtable_t *const assetidremap)
{
    ecs_component_camera_t *const c = cmp_data;

    u64 cmp_idx, member_count;
    sscanf(str_lstrip(lines.views[line_idx], '\t').data,  "%"SCNu64" camera ""%"SCNu64, &cmp_idx, &member_count);

    for (u8 idx = 0; idx < member_count; idx++) 
    {
        const str_t trimmed_line    = str_lstrip(lines.views[line_idx + (idx + 1)], '\t');
        const str_pair_t pair       = str_partition(trimmed_line, ' ');

        if      (str_cmp(pair.pair[0], str("position")))               sscanf(pair.pair[1].data, "%f %f %f", &c->camera.position.x, &c->camera.position.y, &c->camera.position.z);
        else if (str_cmp(pair.pair[0], str("euler_angle")))            sscanf(pair.pair[1].data, "%f %f", &c->camera.euler_angle.x, &c->camera.euler_angle.y);
        else if (str_cmp(pair.pair[0], str("direction_front")))        sscanf(pair.pair[1].data, "%f %f %f", &c->camera.direction.front.x, &c->camera.direction.front.y, &c->camera.direction.front.z);
        else if (str_cmp(pair.pair[0], str("direction_up")))           sscanf(pair.pair[1].data, "%f %f %f", &c->camera.direction.up.x, &c->camera.direction.up.y, &c->camera.direction.up.z);
        else if (str_cmp(pair.pair[0], str("direction_right")))        sscanf(pair.pair[1].data, "%f %f %f", &c->camera.direction.right.x, &c->camera.direction.right.y, &c->camera.direction.right.z);
        else if (str_cmp(pair.pair[0], str("mode")))                   sscanf(pair.pair[1].data, "%d", (int *)&c->mode);
        else if (str_cmp(pair.pair[0], str("follow_orbit_radius")))    sscanf(pair.pair[1].data, "%f", &c->follow.orbit_radius);
        else if (str_cmp(pair.pair[0], str("follow_center_offset")))   sscanf(pair.pair[1].data, "%f %f %f", &c->follow.center_offset.x, &c->follow.center_offset.y, &c->follow.center_offset.z);
        else if (str_cmp(pair.pair[0], str("follow_track_entity_id"))) sscanf(pair.pair[1].data, "%u", &c->follow.track_entity_id);
    }
    return member_count;
}

/* ======================================================================
 *  COLLIDER
 * ==================================================================== */

INTERNAL void ecs_collider_serializer(file_t *const file, const void *const cmp_data)
{
    const ecs_component_collider_t *col = cmp_data;
    char buf[WORD] = {0};

    SERIALIZE_KV(file, buf, "\t", "%i collider 6", ECS_CMP_COLLIDER_IDX);
    SERIALIZE_KV(file, buf, "\t\t", "shape_type %d", col->shape_type);
    SERIALIZE_KV(file, buf, "\t\t", "motion_type %d", col->motion_type);
    SERIALIZE_KV(file, buf, "\t\t", "object_layer_type %u", col->object_layer_type);
    SERIALIZE_KV(file, buf, "\t\t", "dim_half_width %f", col->dim.cube.half_width);
    SERIALIZE_KV(file, buf, "\t\t", "dim_half_height %f", col->dim.cube.half_height);
    SERIALIZE_KV(file, buf, "\t\t", "dim_half_depth %f", col->dim.cube.half_depth);
}

INTERNAL u64 ecs_collider_deserializer(const str_views_t lines, const u64 line_idx, void *const cmp_data, arena_t *const arena, const hashtable_t *const assetidremap)
{
    ecs_component_collider_t *const col = cmp_data;

    u64 cmp_idx, member_count;
    sscanf(str_lstrip(lines.views[line_idx], '\t').data, "%"SCNu64" collider ""%"SCNu64, &cmp_idx, &member_count);

    for (u8 idx = 0; idx < member_count; idx++) 
    {
        const str_t trimmed_line    = str_lstrip(lines.views[line_idx + (idx + 1)], '\t');
        const str_pair_t pair       = str_partition(trimmed_line, ' ');

        if      (str_cmp(pair.pair[0], str("shape_type")))         sscanf(pair.pair[1].data, "%d", (int *)&col->shape_type);
        else if (str_cmp(pair.pair[0], str("motion_type")))        sscanf(pair.pair[1].data, "%d", (int *)&col->motion_type);
        else if (str_cmp(pair.pair[0], str("object_layer_type")))  sscanf(pair.pair[1].data, "%u", &col->object_layer_type);
        else if (str_cmp(pair.pair[0], str("dim_half_width")))     sscanf(pair.pair[1].data, "%f", &col->dim.cube.half_width);
        else if (str_cmp(pair.pair[0], str("dim_half_height")))    sscanf(pair.pair[1].data, "%f", &col->dim.cube.half_height);
        else if (str_cmp(pair.pair[0], str("dim_half_depth")))     sscanf(pair.pair[1].data, "%f", &col->dim.cube.half_depth);
    }

    return member_count;
}
