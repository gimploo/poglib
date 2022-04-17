#pragma once
#include "../ecs/entitymanager.h"
#include "../ds/map.h"
#include "./assetmanager.h"
#include "./action.h"

//NOTE: the action map is a list, i dont like it this way, i might need to make 
//an static list ds of some sort, cuz the extra cycles the input function takes
//is just dumb, it needs to be at O(1). The reason i choose list over map, is 
//that map takes a string as key and not number. 

/*============================================================================
                            - SCENE -
============================================================================*/

#define MAX_ACTIONS_ALLOWED_PER_SCENE 64

typedef struct scene_t {

    const char          *label;
    assetmanager_t      *assets;
    entitymanager_t     manager;
    bool                is_paused;
    bool                is_over;

    // Action map
    list_t              actions;

    void (*init)        (struct scene_t *);
    void (*update)      (struct scene_t *);
    void (*input)       (const action_t );
    void (*render)      (struct scene_t *);
    void (*destroy)     (struct scene_t *);

} scene_t ;


scene_t     scene_init(const char *scene_label);
void        scene_add_action(scene_t *scene, const action_t action);

#define     scene_get_type(PSCENE) (PSCENE)->__enum_id
#define     scene_get_engine(...)   global_poggen; assert(global_poggen)

void        scene_destroy(scene_t *scene);


/*-----------------------------------------------------------------------------*/

#ifndef IGNORE_POGGEN_SCENE_IMPLEMENTATION


void scene_add_action(scene_t *scene, const action_t action)
{
    assert(scene);
    if (scene->input == NULL) eprint("`%s` scene is missing a input() function", scene->label);

    list_t *list = &scene->actions;
    assert(list);

    list_append(list, action);
}


scene_t scene_init(const char *scene_label)
{
    scene_t scene = {
        .label      = scene_label,
        .assets     = NULL,
        .manager    = entitymanager_init(10),
        .is_paused  = false,
        .is_over    = false,
        .actions    = list_init(MAX_ACTIONS_ALLOWED_PER_SCENE, action_t ),

        .init       = NULL,
        .update     = NULL,
        .input      = NULL,
        .render     = NULL,
        .destroy    = NULL 
    };

    return scene;
}


void scene_destroy(scene_t *scene)
{
    assert(scene);

    entitymanager_destroy(&scene->manager);
    scene->destroy(scene);

    scene->assets = NULL;
    scene->label = NULL;
    scene->init  = NULL;
    scene->update = NULL;
    scene->render = NULL;
    scene->destroy = NULL;
    scene->input = NULL;

    list_destroy(&scene->actions);
}

#endif
