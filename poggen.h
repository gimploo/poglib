#pragma once
#include <poglib/application.h>
#include "./poggen/scene.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/concurrency.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/ecs.h"
#include "poglib/physics/jolt-wrapper.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/util/assetmanager.h"

//TODO: collision setup is done during engine init phase - thinking whether we could
//move this over to local scene - gemini says it would hit performance - need to test and figure out

/*=============================================================================
                             - GAME ENGINE -
==============================================================================*/

typedef struct {
    bool                enable_physics;
    bool                enable_ecs;
} poggen_config_t;

typedef struct {
    bool phy_simulation_started;
    physics_sys_jolt_t *instance;
} poggen__internal_physics_t;

typedef struct poggen_t {

    poggen_config_t     config;
    hashtable_t         scenes;
    scene_t             *current_scene;
    bgtask_manager_t    bg_task_manager;
    arena_t             arena;

    struct {
        application_t *app;
    } handle;

    struct {
        poggen__internal_physics_t  physics;
        ecs_t                       ecs;
        assetmanager_t              assets;
        renderqueue_t               renderqueue;
    } systems;

} poggen_t ;

global poggen_t     *global_poggen = NULL;

poggen_t *                          poggen_init(application_t * const app, const poggen_config_t config);
#define                             poggen_add_scene(PGEN, SCENE_NAME)                          poggen__internal_add_scene((PGEN), __impl_scene_init(SCENE_NAME))
void                                poggen_remove_scene(poggen_t *self, str_t label);
void                                poggen_change_scene(poggen_t *self, str_t scene_label);

void                                poggen_register_physics_rules(poggen_t * const self, const physics_sys_jolt_rules_config_t config);
window_t *                          poggen_get_window(const poggen_t *self);
physics_sys_jolt_event_queue_t *    poggen_get_physics_collision_events(const poggen_t * const self);
ecs_t *                             poggen_get_ecs_handle(poggen_t * const self);

void                                poggen_update(poggen_t *self, const f32 dt);
void                                poggen_render(poggen_t *self, const f32 dt);

void                                poggen_destroy(poggen_t *self);


/*----------------------------------------------------------------------------*/

#ifndef IGNORE_POGGEN_IMPLEMENTATION

#define MAX_SCENES_ALLOWED 10



window_t * poggen_get_window(const poggen_t *self)
{
    return application_get_window(self->handle.app);
}

poggen_t * poggen_init(application_t * const app, const poggen_config_t config)
{
    if (!global_window)     eprint("A window is required to run poggen\n");
    if (global_poggen)      eprint("Trying to initialize a second `poggen` in the same instance");

    poggen_t *output = (poggen_t *)calloc(1, sizeof(poggen_t ));
    ASSERT(output);
    arena_t arena = arena_init(&app->handle.arena, 2 * MB);
    *output = (poggen_t ){
        .config             = config,
        .scenes             = hashtable_init(MAX_SCENES_ALLOWED, HT_KEY_TYPE_STR, (ht_value_type){ .size = sizeof(scene_t), .type = HT_STORAGE_BY_REFERENCE }, &app->handle.arena),
        .current_scene      = NULL,
        .arena              = arena,
        .bg_task_manager    = bgtask_manager_init(),
        .handle = {
            .app            = app
        },
        .systems = {
            .assets      = assetmanager_init(&output->bg_task_manager),
            .renderqueue = renderqueue_init(),
            .ecs         = config.enable_ecs ? ecs_init() : (ecs_t){0},
            .physics     = config.enable_physics 
                           ? (poggen__internal_physics_t){
                               .phy_simulation_started = false,
                               .instance = physics_sys_jolt_init(&arena)
                           } : (poggen__internal_physics_t){0},
},
    };

    global_poggen  = output;

    return global_poggen;
}

void poggen__internal_add_scene(poggen_t *self, const scene_t scene_config)
{
    assert(self);

    scene_t * const scene = mem_init((scene_t *)&scene_config, sizeof(scene_t));
    hashtable_insert(
        &self->scenes, 
        (hashtable_key_t){scene_config.label},
        scene
    );

    if (!self->current_scene)
        self->current_scene = scene;

    if (self->config.enable_physics && !self->systems.physics.phy_simulation_started) {
        physics_sys_jolt_start_simulation(self->systems.physics.instance);
        self->systems.physics.phy_simulation_started = true;
    }

    scene->__init(scene);
}


void poggen_remove_scene(poggen_t *self, const str_t label)
{
    assert(self);
    assert(label.len);

    hashtable_delete(&self->scenes, (hashtable_key_t){label});
}

void poggen_change_scene(poggen_t *self, const str_t scene_label)
{
    assert(self);
    assert(scene_label.len);

    const hashtable_t *table = &self->scenes;

    scene_t *scene = (scene_t *)hashtable_get_value(table, (hashtable_key_t){scene_label});
    assert(scene);
    printf("SCENE UPDATED FROM (%s) TO (%s)\n", self->current_scene->label, scene->label);
    self->current_scene = scene;
}

void poggen_render(poggen_t *self, const f32 dt)
{
    ASSERT(self);
    self->current_scene->__render(self->current_scene, dt);
    renderqueue_dispatch(&self->systems.renderqueue);
}

void poggen_update(poggen_t *self, const f32 dt)
{
    assert(self);

    if (self->config.enable_physics && self->systems.physics.instance && !self->systems.physics.phy_simulation_started)
        eprint("Missed to register physics interaction rules, else DISABLE physics in config passed to engine");

    assetmanager_update(&self->systems.assets);
    bgtask_manager_run_all_tasks(&self->bg_task_manager);

    scene_t *current_scene = self->current_scene;
    if (current_scene == NULL) eprint("Current scene is null");

    scene__internal_input_callback(current_scene, dt);

    if (self->systems.physics.phy_simulation_started)
        physics_sys_jolt_update(self->systems.physics.instance, dt);

    if (self->config.enable_ecs) {
        ecs_update(&self->systems.ecs);
    }

    current_scene->__update(current_scene, dt);
}

void poggen_destroy(poggen_t *self)
{
    assert(self);

    assetmanager_destroy(&self->systems.assets);
    hashtable_iterator(&self->scenes, tableentry) {
        hashtable_entry_t *entry = tableentry;
        __scene_destroy(entry->value);
        mem_free(entry->value, sizeof(scene_t));
    }
    hashtable_destroy(&self->scenes);
    bgtask_manager_destroy(&self->bg_task_manager);

    if (self->config.enable_physics)
        physics_sys_jolt_destroy(self->systems.physics.instance);

    if (self->config.enable_ecs)
        ecs_destroy(&self->systems.ecs);

    arena_destroy(&self->arena);

    self->current_scene = NULL;
    free(self);
    self = NULL;
    global_poggen = NULL;
}

void poggen_register_physics_rules(poggen_t * const self, const physics_sys_jolt_rules_config_t config)
{
    ASSERT(self->config.enable_physics);
    ASSERT(self->systems.physics.instance);

    physics_sys_jolt_set_interaction_rules(self->systems.physics.instance, config);
    physics_sys_jolt_start_simulation(self->systems.physics.instance);
}

physics_sys_jolt_event_queue_t * poggen_get_physics_collision_events(const poggen_t * const self)
{
    if (!self->config.enable_physics) {
        eprint("Enable physics first, to use this");
    }

    ASSERT(self->systems.physics.instance);
    return physics_sys_get_collision_event_queue(self->systems.physics.instance);
}

ecs_t * poggen_get_ecs_handle(poggen_t * const self)
{
    ASSERT(self->config.enable_ecs);
    return &self->systems.ecs;
}

#endif
