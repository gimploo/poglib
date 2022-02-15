#pragma once
#include "../ecs/entitymanager.h"
#include "./action.h"

//TODO: Implement actions
/*=========================================
            -- SCENE --
=========================================*/


typedef struct scene_t scene_t ;

scene_t     scene_init(void *engine, const char *scene_label, void (*init)(scene_t *), void (*doaction)(scene_t *, action_t ), void (*update)(scene_t *), void (*render)(scene_t *));
void        scene_register_action(scene_t *scene, action_t action);
void        scene_destroy(scene_t *scene);



#ifndef IGNORE_POGGEN_SCENE_IMPLEMENTATION

#define MAX_ACTIONS_ALLOWED_PER_SCENE 64

struct scene_t {

    const char      *label;
    void            *poggen;
    entitymanager_t manager;
    action_t        actions[SDL_NUM_SCANCODES]; 
    bool            is_paused;
    bool            is_over;

    void (*init)        (struct scene_t *);
    void (*doaction)    (struct scene_t *, action_t );
    void (*update)      (struct scene_t *);
    void (*render)      (struct scene_t *);

};

scene_t scene_init(void *engine, const char *scene_label, void (*init)(scene_t *), void (*doaction)(scene_t *, action_t ), void (*update)(scene_t *), void (*render)(scene_t *))
{
    scene_t scene = {
        .label      = scene_label,
        .poggen     = engine,
        .manager    = entitymanager_init(10),
        .actions    = {0},
        .is_paused  = false,
        .is_over    = false,

        .init       = init,
        .doaction   = doaction,
        .update     = update,
        .render     = render,
    };

    return scene;
}

void scene_add_action(scene_t *scene, action_t action)
{
    assert(scene);

    scene->actions[action.key] = action;
}

void scene_destroy(scene_t *scene)
{
    assert(scene);
    entitymanager_destroy(&scene->manager);
    scene->update = NULL;
    scene->render = NULL;
    scene->init = NULL;
    scene->doaction = NULL;
    scene->label = NULL;
    scene->poggen = NULL;
}

#endif
