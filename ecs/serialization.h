#pragma once
#include "./common.h"
#include "poglib/ecs/component/types.h"
#include "poglib/util/assetmanager.h"

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
    hashtable_iterator(&self->assetid_to_assetpath, iter)
    {
        const hashtable_entry_t *entry  = iter;
        const str_t *const assetpath    = entry->value;
        snprintf(
            buffer.raw_data, buffer.occupied_size, 
            "assetid:%i,assetpath:%*s\n",
            entry->key.u32,
            assetpath->len,
            assetpath->data
        );
        file_writeline(file, buffer.raw_data);
    }
}

INTERNAL void ecs_serializer__internal_write_footer(file_t *const file)
{
    buffer(WORD) buffer = {0};
    snprintf(buffer.raw_data, sizeof(buffer.raw_data), "fin\n");
    file_writeline(file, buffer.raw_data);
}

