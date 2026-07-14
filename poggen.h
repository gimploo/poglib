#pragma once
#include <poglib/application.h>
#include "./poggen/scene.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/concurrency.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/input/commandqueue.h"
#include "poglib/physics/jolt-wrapper.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/util/assetmanager.h"

/*=============================================================================
                             - GAME ENGINE -
==============================================================================*/

typedef struct {
    bool                enable_physics;
} poggen_config_t;

typedef struct {
    bool phy_simulation_started;
    physics_sys_jolt_t *instance;
} poggen__internal_physics_t;

typedef struct poggen_t {

    poggen_config_t     config;
    hashtable_t         scenes;
    scene_t             *current_scene;
    arena_t            *arena;

    struct {
        application_t *app;
    } handle;

    struct {
        poggen__internal_physics_t  physics;
        assetmanager_t              assets;
        renderqueue_t               renderqueue;
        commandqueue_t              commandqueue;
    } systems;

} poggen_t ;

global poggen_t *global_engine = NULL;

poggen_t *                          poggen_init(application_t *const app, const poggen_config_t config);
#define                             poggen_add_scene(PGEN, SCENE_NAME)                          poggen__internal_add_scene((PGEN), scene__internal_init(SCENE_NAME))
void                                poggen_remove_scene(poggen_t *self, str_t label);
void                                poggen_change_scene(poggen_t *self, str_t scene_label);

void                                poggen_register_physics_rules(poggen_t * const self, const physics_sys_jolt_rules_config_t config);
void *                              poggen_get_scene_content(const poggen_t *const self); 
window_t *                          poggen_get_window(const poggen_t *self);
physics_sys_jolt_event_queue_t *    poggen_get_physics_collision_events(const poggen_t * const self);

void                                poggen_update_commandqueue_registry(poggen_t *const self, const commandregistry_t registry);

void                                poggen_tick(poggen_t *const self);
void                                poggen_update(poggen_t *const self);
void                                poggen_render(poggen_t *const self, const f32 dt);

void                                poggen_destroy(poggen_t *const self);


/*----------------------------------------------------------------------------*/

#ifndef IGNORE_POGGEN_IMPLEMENTATION

#define MAX_SCENES_ALLOWED 10


window_t * poggen_get_window(const poggen_t *self)
{
    return application_get_window(self->handle.app);
}

poggen_t * poggen_init(application_t *const app, const poggen_config_t config)
{
    if (!global_window)     eprint("A window is required to run poggen\n");
    if (global_engine)      eprint("Trying to initialize a second `poggen` in the same instance");

    arena_t *arena = arena_init(NULL, 5 * MB);
    global_engine = arena_store(
        arena,
        &(poggen_t ){
            .config             = config,
            .scenes             = hashtable_init(MAX_SCENES_ALLOWED, HT_KEY_TYPE_STR, (ht_value_type){ .size = sizeof(scene_t), .type = HT_STORAGE_BY_REFERENCE }, app->handle.arena),
            .current_scene      = NULL,
            .arena              = arena,
            .handle = {
                .app            = app
            },
            .systems = {
                .assets      = assetmanager_init(global_bgtask_manager),
                .renderqueue = renderqueue_init(),
                .physics     = config.enable_physics 
                               ? (poggen__internal_physics_t){
                                   .phy_simulation_started = false,
                                    .instance = physics_sys_jolt_init(arena)
                               } : (poggen__internal_physics_t){0},
                .commandqueue = {0},
            },
        }, sizeof(poggen_t));

    return global_engine;
}

void poggen__internal_add_scene(poggen_t *const self, const scene_t scene_config)
{
    assert(self);

    scene_t *const scene = mem_init((scene_t *)&scene_config, sizeof(scene_t));
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
    printf("SCENE UPDATED FROM (%s) TO (%s)\n", self->current_scene->label.data, scene->label.data);
    self->current_scene = scene;
}

void poggen_render(poggen_t *const self, const f32 dt)
{
    ASSERT(self);
    self->current_scene->__render(self->current_scene, dt);
    renderqueue_dispatch(&self->systems.renderqueue);
}

void poggen_tick(poggen_t *const self)
{
    ASSERT(self);
    ASSERT(self->current_scene);

    commandqueue_sync(&self->systems.commandqueue);
    scene__internal_tick(self->current_scene);
}

void poggen_update(poggen_t *const self)
{
    const f32 dt = APPLICATION_UPDATE_FIXED_TIME_STEP;
    assert(self);

    if (self->config.enable_physics && self->systems.physics.instance && !self->systems.physics.phy_simulation_started)
        eprint("Missed to register physics interaction rules, else DISABLE physics in config passed to engine");

    assetmanager_update(&self->systems.assets);

    scene_t *current_scene = self->current_scene;
    if (current_scene == NULL) eprint("Current scene is null");

    current_scene->__input(current_scene, dt);

    if (self->systems.physics.phy_simulation_started)
        physics_sys_jolt_update(self->systems.physics.instance, dt);

    scene__internal_update(current_scene, dt);
}

void poggen_destroy(poggen_t *const self)
{
    ASSERT(self);

    hashtable_iterator(&self->scenes, tableentry) {
        hashtable_entry_t *entry = tableentry;
        scene__internal_destroy(entry->value);
        mem_free(entry->value, sizeof(scene_t));
    }
    assetmanager_destroy(&self->systems.assets);
    hashtable_destroy(&self->scenes);

    if (self->config.enable_physics)
        physics_sys_jolt_destroy(self->systems.physics.instance);

    renderqueue_destroy(&self->systems.renderqueue);
    arena_destroy(self->arena);

    global_engine = NULL;
}

void poggen_register_physics_rules(poggen_t *const self, const physics_sys_jolt_rules_config_t config)
{
    ASSERT(self->config.enable_physics);
    ASSERT(self->systems.physics.instance);

    physics_sys_jolt_set_interaction_rules(self->systems.physics.instance, config);
    physics_sys_jolt_start_simulation(self->systems.physics.instance);
}

physics_sys_jolt_event_queue_t * poggen_get_physics_collision_events(const poggen_t *const self)
{
    if (!self->config.enable_physics) {
        eprint("Enable physics first, to use this");
    }

    ASSERT(self->systems.physics.instance);
    return physics_sys_get_collision_event_queue(self->systems.physics.instance);
}

void poggen_update_commandqueue_registry(poggen_t *const self, const commandregistry_t registry)
{
    ASSERT(self);
    commandqueue_update_registry(&self->systems.commandqueue, registry);
}

void * poggen_get_scene_content(const poggen_t *const self)
{
    return self->current_scene->content;
}

#endif
