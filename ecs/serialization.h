#pragma once
#include "./common.h"
#include "poglib/ecs/component/types.h"
#include "poglib/util/assetmanager.h"

INTERNAL const struct {
    ecs_component_type type;
    u8 idx;
    const char *prefix;
    u8 prefix_len;
} ecs_deserializer__internal_cmp_prefix_map[ECS_CMP_COUNT] = {
    { ECS_CMP_TRANSFORM, ECS_CMP_TRANSFORM_IDX, "transform:", 10 },
    { ECS_CMP_MODEL,     ECS_CMP_MODEL_IDX,     "model:",     6  },
    { ECS_CMP_INPUT,     ECS_CMP_INPUT_IDX,     "input:",     6  },
    { ECS_CMP_MATERIAL,  ECS_CMP_MATERIAL_IDX,  "material:",  9  },
    { ECS_CMP_CAMERA,    ECS_CMP_CAMERA_IDX,    "camera:",    7  },
    { ECS_CMP_COLLIDER,  ECS_CMP_COLLIDER_IDX,  "collider:",  9  },
    { ECS_CMP_MESH,      ECS_CMP_MESH_IDX,      "mesh:",      5  },
};

INTERNAL void ecs_serializer__internal_write_header(file_t *const file)
{
    const u32 magic        = ECS_SAVE_FILE_MAGIC;
    const u32 version      = ECS_SAVE_FILE_VERSION;

    buffer(WORD) buffer = {0};
    snprintf(buffer.raw_data, sizeof(buffer.raw_data), "ECS v%i save\n", version);
    file_writeline(file, (char *)buffer.raw_data);
}

INTERNAL void ecs_serializer__internal_write_entity_data(file_t *const file, const u32 entity_id, const u32 signature)
{
    const char *format = ""
    "entity:%lu\n"
    "component_signature:%lu\n";

    buffer(WORD) buffer = {0};
    snprintf(buffer.raw_data, sizeof(buffer.raw_data), format, entity_id, signature);
    file_writeline(file, (char *)buffer.raw_data);
}

INTERNAL void ecs_serializer__internal_entity_cmp_data(file_t *const file, ecs_component_type type, void *cmp_data)
{
    buffer(WORD) buffer = {0};

    switch(type)
    {
        case ECS_CMP_TRANSFORM: {
            const ecs_component_transform_t *const transform = cmp_data;
            snprintf(
                buffer.raw_data, 
                sizeof(buffer.raw_data),

                "transform:{totalmembers:%lu,bytesize:%lu}\n"
                "\tposition:[%f,%f,%f]\n"
                "\torientation:[%f,%f,%f,%f]\n"
                "\tscale:[%f,%f,%f]\n"
                "\tvelocity:[%f,%f,%f]\n"
                "\tsource:%i\n",

                5, sizeof(ecs_component_transform_t),
                transform->position.x, transform->position.y, transform->position.z,
                transform->orientation.x, transform->orientation.y, transform->orientation.z, transform->orientation.w,
                transform->scale.x, transform->scale.y, transform->scale.z,
                transform->velocity.x, transform->velocity.y, transform->velocity.z,
                transform->source
            );
        } break;
        case ECS_CMP_MODEL: {
            const ecs_component_model_t *const model = cmp_data;
            snprintf(
                buffer.raw_data, 
                sizeof(buffer.raw_data),

                "model:{totalmembers:%lu,bytesize:%lu}\n"
                "\tasset_id:%lu\n",

                2, sizeof(ecs_component_model_t),
                model->asset_id
            );
        } break;
        case ECS_CMP_INPUT: {
            const ecs_component_input_t *const input = cmp_data;
            snprintf(
                buffer.raw_data,
                sizeof(buffer.raw_data),

                "input:{totalmembers:%lu,bytesize:%lu}\n"
                "\tdirection_source:%i\n"
                "\tinput_behavior:%p\n"
                "\tstate.position:[%f,%f,%f]\n"
                "\tstate.orientation:[%f,%f,%f,%f]\n"
                "\tstate.front:[%f,%f,%f]\n"
                "\tstate.right:[%f,%f,%f]\n",

                3, sizeof(ecs_component_input_t),
                input->direction_source,
                (void *)input->input_behavior,
                input->internal.state.current_position.x, input->internal.state.current_position.y, input->internal.state.current_position.z,
                input->internal.state.current_orientation.x, input->internal.state.current_orientation.y, input->internal.state.current_orientation.z, input->internal.state.current_orientation.w,
                input->internal.state.front.x, input->internal.state.front.y, input->internal.state.front.z,
                input->internal.state.right.x, input->internal.state.right.y, input->internal.state.right.z
            );
        } break;
        case ECS_CMP_MATERIAL: {
            const ecs_component_material_t *const material = cmp_data;
            snprintf(
                buffer.raw_data, 
                sizeof(buffer.raw_data),
                "material:{totalmembers:%lu,bytesize:%lu}\n"
                "\ttexture_asset_id:%u\n"
                "\tshader_asset_id:%u\n",
                2, sizeof(ecs_component_material_t), material->texture_asset_id, material->shader_asset_id
            );
        } break;
       case ECS_CMP_CAMERA: {
            const ecs_component_camera_t *const camera = cmp_data;
            snprintf(
                buffer.raw_data,
                sizeof(buffer.raw_data),

                "camera:{totalmembers:%lu,bytesize:%lu}\n"
                "\tposition:[%f,%f,%f]\n"
                "\teuler_angle:[%f,%f]\n"
                "\tdirection.front:[%f,%f,%f]\n"
                "\tdirection.up:[%f,%f,%f]\n"
                "\tdirection.right:[%f,%f,%f]\n"
                "\tmode:%i\n"
                "\tfollow.orbit_radius:%f\n"
                "\tfollow.center_offset:[%f,%f,%f]\n"
                "\tfollow.track_entity_id:%u\n",

                3, sizeof(ecs_component_camera_t),

                camera->camera.position.x, camera->camera.position.y, camera->camera.position.z,
                camera->camera.euler_angle.x, camera->camera.euler_angle.y,
                camera->camera.direction.front.x, camera->camera.direction.front.y, camera->camera.direction.front.z,
                camera->camera.direction.up.x, camera->camera.direction.up.y, camera->camera.direction.up.z,
                camera->camera.direction.right.x, camera->camera.direction.right.y, camera->camera.direction.right.z,
                camera->mode,
                camera->follow.orbit_radius,
                camera->follow.center_offset.x, camera->follow.center_offset.y, camera->follow.center_offset.z,
                camera->follow.track_entity_id
            );
        } break;
        case ECS_CMP_COLLIDER: {
            const ecs_component_collider_t *const collider = cmp_data;
            snprintf(
                buffer.raw_data,
                sizeof(buffer.raw_data),

                "collider:{totalmembers:%lu,bytesize:%lu}\n"
                "\tshape_type:%i\n"
                "\tmotion_type:%i\n"
                "\tobject_layer_type:%lu\n"
                "\tdim.cube:[%f,%f,%f]\n"
                "\tdim.sphere:[%f]\n"
                "\tdim.capsule:[%f,%f]\n",

                5, sizeof(ecs_component_collider_t),
                collider->shape_type,
                collider->motion_type,
                (unsigned long)collider->object_layer_type,
                collider->dim.cube.half_width, collider->dim.cube.half_height, collider->dim.cube.half_depth,
                collider->dim.sphere.radius,
                collider->dim.capsule.radius, collider->dim.capsule.half_height
            );
        } break;
        case ECS_CMP_MESH: {
            const ecs_component_mesh_t *const mesh = cmp_data;
            snprintf(
                buffer.raw_data,
                sizeof(buffer.raw_data),

                "mesh:{totalmembers:%lu,bytesize:%lu}\n"
                "\tasset_id:%lu\n"
                "\tprototype_sprite_type:%i\n",

                2, sizeof(ecs_component_mesh_t),
                mesh->asset_id,
                mesh->prototype_sprite_type
            );
        } break;

    }
    file_writeline(file, (char *)buffer.raw_data);
}


void assetmanager_serialize_to_file(const assetmanager_t *const self, file_t *const file)
{
    ASSERT(!file->is_closed);

    buffer(WORD) buffer = {0};
    hashtable_iterator(&self->assetmeta_lookup, iter)
    {
        const hashtable_entry_t *entry  = iter;
        const str_t *const assetpath    = entry->value;
        snprintf(
            buffer.raw_data, buffer.size, 
            "assetid:%i,assetpath:%*s\n",
            entry->key.u32,
            assetpath->len,
            assetpath->data
        );
        file_writeline(file, buffer.raw_data);
    }
}

INTERNAL void ecs_deserializer__internal_parse_cmp_data_line(
    const char *const line,
    const ecs_component_type type,
    void *const cmp_data)
{
    switch (type)
    {
        case ECS_CMP_TRANSFORM: {
            ecs_component_transform_t *t = cmp_data;
            if      (sscanf(line, "\tposition:[%f,%f,%f]",       &t->position.x,      &t->position.y,      &t->position.z)      == 3) return;
            else if (sscanf(line, "\torientation:[%f,%f,%f,%f]",  &t->orientation.x,   &t->orientation.y,   &t->orientation.z,   &t->orientation.w) == 4) return;
            else if (sscanf(line, "\tscale:[%f,%f,%f]",           &t->scale.x,         &t->scale.y,         &t->scale.z)        == 3) return;
            else if (sscanf(line, "\tvelocity:[%f,%f,%f]",        &t->velocity.x,       &t->velocity.y,       &t->velocity.z)     == 3) return;
            else if (sscanf(line, "\tsource:%i",                  &t->source)                                                          == 1) return;
        } break;
        case ECS_CMP_MODEL: {
            ecs_component_model_t *m = cmp_data;
            sscanf(line, "\tasset_id:%lu", &m->asset_id);
        } break;
        case ECS_CMP_INPUT: {
            ecs_component_input_t *in = cmp_data;
            if      (sscanf(line, "\tdirection_source:%i",        &in->direction_source)                                               == 1) return;
            else if (strncmp(line, "\tinput_behavior:", 17) == 0) { in->input_behavior = NULL; }
            else if (sscanf(line, "\tstate.position:[%f,%f,%f]",     &in->internal.state.current_position.x,    &in->internal.state.current_position.y,    &in->internal.state.current_position.z)    == 3) return;
            else if (sscanf(line, "\tstate.orientation:[%f,%f,%f,%f]",&in->internal.state.current_orientation.x, &in->internal.state.current_orientation.y, &in->internal.state.current_orientation.z, &in->internal.state.current_orientation.w) == 4) return;
            else if (sscanf(line, "\tstate.front:[%f,%f,%f]",        &in->internal.state.front.x,               &in->internal.state.front.y,               &in->internal.state.front.z)               == 3) return;
            else if (sscanf(line, "\tstate.right:[%f,%f,%f]",        &in->internal.state.right.x,               &in->internal.state.right.y,               &in->internal.state.right.z)               == 3) return;
        } break;
        case ECS_CMP_MATERIAL: {
            ecs_component_material_t *mat = cmp_data;
            if      (sscanf(line, "\ttexture_asset_id:%u", &mat->texture_asset_id) == 1) return;
            else if (sscanf(line, "\tshader_asset_id:%u",  &mat->shader_asset_id)  == 1) return;
        } break;
        case ECS_CMP_CAMERA: {
            ecs_component_camera_t *cam = cmp_data;
            if      (sscanf(line, "\tposition:[%f,%f,%f]",          &cam->camera.position.x,       &cam->camera.position.y,       &cam->camera.position.z)       == 3) return;
            else if (sscanf(line, "\teuler_angle:[%f,%f]",           &cam->camera.euler_angle.x,    &cam->camera.euler_angle.y)                    == 2) return;
            else if (sscanf(line, "\tdirection.front:[%f,%f,%f]",    &cam->camera.direction.front.x,&cam->camera.direction.front.y,&cam->camera.direction.front.z)== 3) return;
            else if (sscanf(line, "\tdirection.up:[%f,%f,%f]",       &cam->camera.direction.up.x,   &cam->camera.direction.up.y,   &cam->camera.direction.up.z)   == 3) return;
            else if (sscanf(line, "\tdirection.right:[%f,%f,%f]",    &cam->camera.direction.right.x,&cam->camera.direction.right.y,&cam->camera.direction.right.z)== 3) return;
            else if (sscanf(line, "\tmode:%i",                       &cam->mode)                                                                    == 1) return;
            else if (sscanf(line, "\tfollow.orbit_radius:%f",        &cam->follow.orbit_radius)                                                     == 1) return;
            else if (sscanf(line, "\tfollow.center_offset:[%f,%f,%f]",&cam->follow.center_offset.x,  &cam->follow.center_offset.y,  &cam->follow.center_offset.z)  == 3) return;
            else if (sscanf(line, "\tfollow.track_entity_id:%u",     &cam->follow.track_entity_id)                                                  == 1) return;
        } break;
        case ECS_CMP_COLLIDER: {
            ecs_component_collider_t *col = cmp_data;
            if      (sscanf(line, "\tshape_type:%i",             &col->shape_type)                                                            == 1) return;
            else if (sscanf(line, "\tmotion_type:%i",            &col->motion_type)                                                           == 1) return;
            else if (sscanf(line, "\tobject_layer_type:%lu",     (unsigned long *)&col->object_layer_type)                                    == 1) return;
            else if (sscanf(line, "\tdim.cube:[%f,%f,%f]",       &col->dim.cube.half_width,    &col->dim.cube.half_height,    &col->dim.cube.half_depth)     == 3) return;
            else if (sscanf(line, "\tdim.sphere:[%f]",           &col->dim.sphere.radius)                                                     == 1) return;
            else if (sscanf(line, "\tdim.capsule:[%f,%f]",       &col->dim.capsule.radius,     &col->dim.capsule.half_height)                                 == 2) return;
        } break;
        case ECS_CMP_MESH: {
            ecs_component_mesh_t *mesh = cmp_data;
            if      (sscanf(line, "\tasset_id:%lu",              &mesh->asset_id)                                                             == 1) return;
            else if (sscanf(line, "\tprototype_sprite_type:%i",  &mesh->prototype_sprite_type)                                                == 1) return;
        } break;
    }
}

INTERNAL void ecs_serializer__internal_write_footer(file_t *const file)
{
    buffer(WORD) buffer = {0};
    snprintf(buffer.raw_data, sizeof(buffer.raw_data), "fin\n");
    file_writeline(file, (char *)buffer.raw_data);
}

