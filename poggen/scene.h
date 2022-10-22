#pragma once
#include "../ecs/entitymanager.h"
#include "../util/assetmanager.h"
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

    const char       *label;
    assetmanager_t   *assets;
    entitymanager_t  manager;

    void             *content;
    
    list_t           __actions;                     // Action map
    bool             __is_paused;
    bool             __is_over;
    void             (*__init)(struct scene_t *);
    void             (*__update)(struct scene_t *, const f32 );
    void             (*__input)(const action_t );
    void             (*__render)(struct scene_t *);
    void             (*__destroy)(struct scene_t *);

} scene_t ;


void                scene_register_action(scene_t *scene, const action_t action);
void                scene_pass_content(scene_t *self, const void *content, const u64 content_size);

#define             scene_get_type(PSCENE)                                     (PSCENE)->__enum_id
#define             scene_get_engine(...)                                      global_poggen



/*-----------------------------------------------------------------------------*/

#ifndef IGNORE_POGGEN_SCENE_IMPLEMENTATION

void scene_pass_content(scene_t *self, const void *content, const u64 content_size)
{
    assert(content);

    self->content = calloc(1, content_size);
    assert(self->content);
    memcpy(self->content, content, content_size);
}

void scene_register_action(scene_t *scene, const action_t action)
{
    assert(scene);
    if (scene->__input == NULL) eprint("`%s` scene is missing a input() function", scene->label);

    list_t *list = &scene->__actions;
    assert(list);

    list_append(list, action);
}

#define __impl_scene_init(SCENE_NAME)\
    (scene_t ){\
        .label          = #SCENE_NAME,\
        .assets         = NULL,\
        .manager        = entitymanager_init(10),\
        .content        = NULL,\
        .__actions      = list_init(action_t ),\
        .__is_paused    = false,\
        .__is_over      = false,\
        .__init         = SCENE_NAME##_init,\
        .__update       = SCENE_NAME##_update,\
        .__input        = SCENE_NAME##_input,\
        .__render       = SCENE_NAME##_render,\
        .__destroy      = SCENE_NAME##_destroy,\
   }



void __scene_destroy(scene_t *scene)
{
    assert(scene);

    entitymanager_destroy(&scene->manager);
    scene->__destroy(scene);

    scene->assets = NULL;
    scene->label = NULL;
    scene->__init  = NULL;
    scene->__update = NULL;
    scene->__render = NULL;
    scene->__destroy = NULL;
    scene->__input = NULL;

    if (scene->content) {
        free(scene->content);
        scene->content = NULL;
    }

    list_destroy(&scene->__actions);
}

#endif
