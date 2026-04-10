#pragma once
#include "./util/assetmanager.h"
#include "./poggen/scene.h"
#include "poggen/action.h"
#include "poglib/basic/arena.h"
#include "poglib/physics/jolt-wrapper.h"

//TODO: collision setup is done during engine init phase - thinking whether we could
//move this over to local scene - gemini says it would hit performance - need to test and figure out

/*=============================================================================
                             - GAME ENGINE -
==============================================================================*/

typedef struct {
    bool enable_physics;
} poggen_config_t;

typedef struct {
    bool phy_simulation_started;
    physics_sys_jolt_t *instance;
} poggen__internal_physics_t;

typedef struct poggen_t {

    poggen_config_t     config;
    assetmanager_t      assets;
    hashtable_t         scenes;
    scene_t             *current_scene;
    bgtask_manager_t    bg_task_manager;
    arena_t             arena;

    struct {
        application_t *const app;
    } handle;

    poggen__internal_physics_t physics_sys;

} poggen_t ;

global poggen_t     *global_poggen = NULL;

poggen_t *                          poggen_init(application_t * const app, const poggen_config_t config);
#define                             poggen_add_scene(PGEN, SCENE_NAME)                          __impl_poggen_add_scene((PGEN), __impl_scene_init(SCENE_NAME))
void                                poggen_remove_scene(poggen_t *self, const char *label);
void                                poggen_change_scene(poggen_t *self, const char *scene_label);

void                                poggen_register_physics_rules(poggen_t * const self, const physics_sys_jolt_rules_config_t config);
window_t *                          poggen_get_window(const poggen_t *self);
physics_sys_jolt_event_queue_t *    poggen_get_physics_collision_events(const poggen_t * const self);

void                                poggen_update(poggen_t *self, const f32 dt);
#define                             poggen_render(PGEN, DT)                                         (PGEN)->current_scene->__render((PGEN)->current_scene, DT)

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

    arena_t arena = arena_init(&app->handle.arena, 2 * MB);

    poggen_t tmp =  (poggen_t ){
        .config         = config,
        .assets         = assetmanager_init(),
        .scenes         = hashtable_init(MAX_SCENES_ALLOWED, scene_t ),
        .current_scene  = NULL,
        .arena          = arena,
        .bg_task_manager = bgtask_manager_init(),
        .handle = {
            .app          = app
        },
        .physics_sys = config.enable_physics ? (poggen__internal_physics_t){
            .instance = physics_sys_jolt_init(&arena),
            .phy_simulation_started = false
        } : (poggen__internal_physics_t){0}
    };
    poggen_t *output = (poggen_t *)calloc(1, sizeof(poggen_t ));
    memcpy(output, &tmp, sizeof(tmp));

    ASSERT(output);

    global_poggen  = output;

    return global_poggen;
}

void __impl_poggen_add_scene(poggen_t *self, const scene_t scene)
{
    assert(self);
    scene_t *tmp = (scene_t *)hashtable_insert(
        &self->scenes, 
        scene.label, 
        mem_init((scene_t *)&scene, sizeof(scene_t))
    );

    if (!self->current_scene)
        self->current_scene = tmp;

    if (self->config.enable_physics && !self->physics_sys.phy_simulation_started) {
        physics_sys_jolt_start_simulation(self->physics_sys.instance);
        self->physics_sys.phy_simulation_started = true;
    }

    tmp->__init(tmp);
}


void poggen_remove_scene(poggen_t *self, const char *label)
{
    assert(self);
    assert(label);

    hashtable_delete(&self->scenes, label);
}

void poggen_change_scene(poggen_t *self, const char *scene_label)
{
    assert(self);
    assert(scene_label);

    const hashtable_t *table = &self->scenes;

    scene_t *scene = (scene_t *)hashtable_get_value(table, scene_label);
    assert(scene);
    printf("SCENE UPDATED FROM (%s) TO (%s)\n", self->current_scene->label, scene->label);
    self->current_scene = scene;
}

void poggen_update(poggen_t *self, const f32 dt)
{
    assert(self);

    if (self->config.enable_physics && self->physics_sys.instance && !self->physics_sys.phy_simulation_started)
        eprint("Missed to register physics interaction rules, else DISABLE physics in config passed to engine");

    bgtask_manager_run_all_tasks(&self->bg_task_manager);

    scene_t *current_scene = self->current_scene;
    if (current_scene == NULL) eprint("Current scene is null");

    current_scene->__input(current_scene, dt);

    if (self->physics_sys.phy_simulation_started)
        physics_sys_jolt_update(self->physics_sys.instance, dt);
    current_scene->__update(current_scene, dt);
}

void poggen_destroy(poggen_t *self)
{
    assert(self);

    assetmanager_destroy(&self->assets);
    hashtable_iterator(&self->scenes, entry) {
        __scene_destroy((scene_t *)hashtable_get_entry_value(&self->scenes, entry));
        mem_free((void *)hashtable_get_entry_value(&self->scenes, entry), sizeof(scene_t));
    }
    hashtable_destroy(&self->scenes);
    bgtask_manager_destroy(&self->bg_task_manager);

    if (self->config.enable_physics)
        physics_sys_jolt_destroy(self->physics_sys.instance);

    arena_destroy(&self->arena);

    self->current_scene = NULL;
    free(self);
    self = NULL;
    global_poggen = NULL;
}

void poggen_register_physics_rules(poggen_t * const self, const physics_sys_jolt_rules_config_t config)
{
    ASSERT(self->config.enable_physics);
    ASSERT(self->physics_sys.instance);

    physics_sys_jolt_set_interaction_rules(self->physics_sys.instance, config);
    physics_sys_jolt_start_simulation(self->physics_sys.instance);
}

physics_sys_jolt_event_queue_t * poggen_get_physics_collision_events(const poggen_t * const self)
{
    if (!self->config.enable_physics) {
        eprint("Enable physics first, to use this");
    }

    ASSERT(self->physics_sys.instance);
    return physics_sys_get_collision_event_queue(self->physics_sys.instance);
}

#endif
