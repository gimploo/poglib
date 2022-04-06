#pragma once
#include "./application.h"
#include "./ecs/entitymanager.h"
#include "./poggen/assetmanager.h"
#include "./poggen/scene.h"
#include "./ecs/systems.h"

//TODO: Actions

/*=====================================
            - GAME ENGINE -
=====================================*/


typedef struct poggen_t poggen_t ;

global poggen_t *global_poggen = NULL;

poggen_t        poggen_init(window_t *);

void            poggen_add_scene(poggen_t *self, const char *label, const scene_t scene);
void            poggen_remove_scene(poggen_t *self, const char *label);
void            poggen_change_scene(poggen_t *self, const char *scene_label);

void            poggen_destroy(poggen_t *self);



#ifndef IGNORE_POGGEN_IMPLEMENTATION

#define MAX_SCENES_ALLOWED 10


struct poggen_t {

    window_t        *window;
    assetmanager_t  assets;
    hashtable_t     scenes;
    scene_t         *current_scene;

    // ecs systems
    s_renderer2d_t  renderer2d;
};

poggen_t poggen_init(window_t *window)
{
    return (poggen_t ) {
        .window         = window,
        .assets         = assetmanager_init(),
        .scenes         = hashtable_init(MAX_SCENES_ALLOWED, scene_t ),
        .current_scene  = NULL,

        .renderer2d = s_renderer2d_init(),
    };
}


void poggen_add_scene(poggen_t *self, const char *label, scene_t scene)
{
    assert(self);
    assert(label);

    global_poggen = self;

    hashtable_insert(&self->scenes, label, scene);
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

    hashtable_t *table = &self->scenes;

    scene_t *scene = (scene_t *)hashtable_get_value_by_key(table, scene_label);
    if (!scene) 
        eprint("SCENE (%s) NOT FOUND", scene_label);

    printf("SCENE UPDATED FROM (%s) TO (%s)\n", self->current_scene->label, scene->label);
    self->current_scene = scene;
}


void poggen_destroy(poggen_t *self)
{
    assert(self);

    self->window = NULL;
    assetmanager_destroy(&self->assets);
    hashtable_destroy(&self->scenes);
    self->current_scene = NULL;

    global_poggen = NULL;
    s_renderer2d_destroy(&self->renderer2d);

}

#endif
