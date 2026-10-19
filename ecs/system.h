#pragma once
#include "./common.h"
#include "poglib/ecs/systems/animation.h"
#include "poglib/ecs/systems/camera.h"
#include "poglib/ecs/systems/collider.h"
#include "poglib/ecs/systems/directional_light.h"
#include "poglib/ecs/systems/input.h"
#include "poglib/ecs/systems/light.h"
#include "poglib/ecs/systems/mesh.h"
#include "poglib/ecs/systems/model.h"
#include "poglib/ecs/systems/transform.h"
#include "poglib/ecs/systems/material.h"

void ecs_add_system(ecs_t *const self, const ecs_system_t system)
{
    ASSERT(self);
    ASSERT(system.callback);

    ecs_systemmanager_t *const x = &self->managers.systemmanager;
    ASSERT(x->count < ECS_SYSTEM_MAX_COUNT);

    x->systems[x->count++] = system;
}

void ecs_add_all_core_systems(ecs_t *const self)
{
    ecs_add_system(
        self, 
        (ecs_system_t) {
            .callback = ecs_system_camera
        }
    );

    ecs_add_system(
        self,
        (ecs_system_t) {
            .callback = ecs_system_input
        }
    );

    ecs_add_system(
        self, 
        (ecs_system_t) {
            .callback = ecs_system_transform
        }
    );

    ecs_add_system(
        self, 
        (ecs_system_t) {
            .callback = ecs_system_collider
        }
    );

    ecs_add_system(
        self, 
        (ecs_system_t) {
            .callback = ecs_system_upload_light_data_to_gpu
        }
    );

    ecs_add_system(
        self, 
        (ecs_system_t) {
            .callback = ecs_system_material
        }
    );


    //FIXME: reason - is that cloth simulation needs to happen after
    //animation and before which had been eariler. since ecs_update happen
    //after scene_update
#if 0
    ecs_add_system(
        self, 
        (ecs_system_t) {
            .callback = ecs_system_animation
        }
    );
#endif

    ecs_add_system(
        self, 
        (ecs_system_t) {
            .callback = ecs_system_render_model
        }
    );

    ecs_add_system(
        self, 
        (ecs_system_t) {
            .callback = ecs_system_render_mesh
        }
    );

    ecs_add_system(
        self, 
        (ecs_system_t) {
            .callback = ecs_system_directional_light
        }
    );
}


