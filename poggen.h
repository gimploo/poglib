#pragma once
#include "./util/assetmanager.h"
#include "./poggen/scene.h"
#include "poggen/action.h"

/*=============================================================================
                             - GAME ENGINE -
==============================================================================*/

typedef struct poggen_t {

    assetmanager_t      assets;
    hashtable_t               scenes;
    scene_t             *current_scene;

    struct {
        const application_t *const app;
    } handle;

} poggen_t ;

global poggen_t     *global_poggen = NULL;

poggen_t *          poggen_init(const application_t * const app);
#define             poggen_add_scene(PGEN, SCENE_NAME)                          __impl_poggen_add_scene((PGEN), __impl_scene_init(SCENE_NAME))
void                poggen_remove_scene(poggen_t *self, const char *label);
void                poggen_change_scene(poggen_t *self, const char *scene_label);

window_t *          poggen_get_window(const poggen_t *self);

void                poggen_update(poggen_t *self, const f32 dt);
#define             poggen_render(PGEN)                                         (PGEN)->current_scene->__render((PGEN)->current_scene)

void                poggen_destroy(poggen_t *self);


/*----------------------------------------------------------------------------*/

#ifndef IGNORE_POGGEN_IMPLEMENTATION

#define MAX_SCENES_ALLOWED 10


window_t * poggen_get_window(const poggen_t *self)
{
    return application_get_window(self->handle.app);
}

poggen_t * poggen_init(const application_t * const app)
{
    if (!global_window)     eprint("A window is required to run poggen\n");
    if (global_poggen)      eprint("Trying to initialize a second `poggen` in the same instance");

    poggen_t tmp =  (poggen_t ){
        .assets         = assetmanager_init(),
        .scenes         = hashtable_init(MAX_SCENES_ALLOWED, scene_t ),
        .current_scene  = NULL,
        .handle = {
            .app          = app
        }
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

    scene_t *current_scene = self->current_scene;
    if (current_scene == NULL) eprint("Current scene is null");

    window_update_user_input(self->handle.app->handle.window);

    current_scene->__input(current_scene, dt);
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

    self->current_scene = NULL;


    free(self);
    self = NULL;

    global_poggen = NULL;

}

#endif
