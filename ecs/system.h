#pragma once
#include "./common.h"
#include "poglib/ecs/systems/animation.h"
#include "poglib/ecs/systems/camera.h"
#include "poglib/ecs/systems/collider.h"
#include "poglib/ecs/systems/input.h"
#include "poglib/ecs/systems/mesh.h"
#include "poglib/ecs/systems/model.h"
#include "poglib/ecs/systems/transform.h"

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
            .callback = ecs_system_animation
        }
    );


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
}


