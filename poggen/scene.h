#pragma once
#include "../ecs/entitymanager.h"
#include "./action.h"

/*=========================================
            -- SCENE --
=========================================*/


typedef struct scene_t scene_t ;

scene_t     scene_init(void *engine, const char *scene_label, void (*do_actions)(scene_t *), void (*update)(scene_t *), void (*render)(scene_t *));
void        scene_add_action(scene_t *scene, action_t action);
void        scene_destroy(scene_t *scene);



#ifndef IGNORE_POGGEN_SCENE_IMPLEMENTATION

#define MAX_ACTIONS_ALLOWED_PER_SCENE 64

struct scene_t {

    const char      *label;
    void            *poggen;
    entitymanager_t manager;
    list_t          actions; 
    bool            is_paused;
    bool            is_over;

    void (*input) (struct scene_t *);
    void (*update) (struct scene_t *);
    void (*render) (struct scene_t *);

};

scene_t scene_init(void *engine, const char *scene_label, void (*input)(scene_t *), void (*update)(scene_t *), void (*render)(scene_t *))
{
    scene_t scene = {
        .label      = scene_label,
        .poggen     = engine,
        .manager    = entitymanager_init(MAX_ENTITIES_ALLOWED_TO_BE_CREATED_PER_FRAME),
        .actions    = list_init(4, action_t ), 
        .is_paused  = false,
        .is_over    = false,

        .input      = input,
        .update     = update,
        .render     = render,
    };

    return scene;
}

void scene_add_action(scene_t *scene, action_t action)
{
    assert(scene);

    list_append(&scene->actions, action);
}

void scene_destroy(scene_t *scene)
{
    assert(scene);
    entitymanager_destroy(&scene->manager);
    list_destroy(&scene->actions);
    scene->update = NULL;
    scene->render = NULL;
    scene->label = NULL;
    scene->poggen = NULL;
}

#endif
