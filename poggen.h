#pragma once
#include "./util/assetmanager.h"
#include "./poggen/scene.h"

//TODO: Actions

/*=============================================================================
                             - GAME ENGINE -
==============================================================================*/

typedef struct poggen_t {

    assetmanager_t      assets;
    map_t               scenes;
    scene_t             *current_scene;

    window_t            *__window;

} poggen_t ;

global poggen_t     *global_poggen = NULL;

poggen_t *          poggen_init(void);
#define             poggen_add_scene(PGEN, SCENE_NAME)          __impl_poggen_add_scene(PGEN, SCENE_NAME)
void                poggen_remove_scene(poggen_t *self, const char *label);
void                poggen_change_scene(poggen_t *self, const char *scene_label);

void                poggen_doactions_currenscene(poggen_t *self);          
#define             poggen_update_currentscene(PGEN)            (PGEN)->current_scene->update((PGEN)->current_scene)
#define             poggen_render_currentscene(PGEN)            (PGEN)->current_scene->render((PGEN)->current_scene)

void                poggen_destroy(poggen_t *self);


/*----------------------------------------------------------------------------*/

#ifndef IGNORE_POGGEN_IMPLEMENTATION

#define MAX_SCENES_ALLOWED 10



poggen_t * poggen_init(void)
{
    if (!global_window) eprint("A window is required to run poggen\n");

    if (global_poggen)
        eprint("Trying to initialize a second `poggen` in the same instance");

    poggen_t tmp =  {
        .assets         = assetmanager_init(),
        .scenes         = map_init(MAX_SCENES_ALLOWED, scene_t ),
        .current_scene  = NULL,
        .__window       = global_window
    };

    poggen_t *output = (poggen_t *)calloc(1, sizeof(poggen_t ));
    assert(output);


    *output = tmp;

    global_poggen = output;

    return output;
}


#define __impl_poggen_add_scene(PGEN, SCENE_NAME) do {\
\
    scene_t __TMP##SCENE_NAME = scene_init(#SCENE_NAME);\
\
    __TMP##SCENE_NAME.init      = SCENE_NAME##_init;\
    __TMP##SCENE_NAME.update    = SCENE_NAME##_update;\
    __TMP##SCENE_NAME.input     = SCENE_NAME##_input;\
    __TMP##SCENE_NAME.render    = SCENE_NAME##_render;\
    __TMP##SCENE_NAME.destroy   = SCENE_NAME##_destroy;\
\
    (__TMP##SCENE_NAME).assets              = &(PGEN)->assets;\
\
    if (!(PGEN)->current_scene)\
        (PGEN)->current_scene = map_insert(&(PGEN)->scenes, #SCENE_NAME, __TMP##SCENE_NAME);\
    else\
        map_insert(&(PGEN)->scenes, #SCENE_NAME, __TMP##SCENE_NAME);\
\
} while (0)


void poggen_remove_scene(poggen_t *self, const char *label)
{
    assert(self);
    assert(label);

    map_delete(&self->scenes, label);
}

void poggen_change_scene(poggen_t *self, const char *scene_label)
{
    assert(self);
    assert(scene_label);

    map_t *table = &self->scenes;

    scene_t *scene = (scene_t *)map_get_value(table, scene_label);
    assert(scene);

    printf("SCENE UPDATED FROM (%s) TO (%s)\n", self->current_scene->label, scene->label);
    self->current_scene = scene;
}

void poggen_doactions_currenscene(poggen_t *self)
{
    assert(self);

    window_t *win = self->__window;
    assert(win);

    SDL_Event *event = &win->__sdl_event;
    assert(event);
    
    while(SDL_PollEvent(event) > 0) 
    {
        if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
        {
            scene_t *scene = self->current_scene;
            list_t *actions = &scene->actions;

            list_iterator(actions, i) {

                action_t *action = (action_t *)i; assert(i); 

                if (action->key == event->key.keysym.scancode) {
                    action->__is_active = true;
                    scene->input(*action); 
                } else {
                    action->__is_active = false;
                }
            }
        }
    }

}

void poggen_destroy(poggen_t *self)
{
    assert(self);

    assetmanager_destroy(&self->assets);

    map_t *map = &self->scenes;

    map_iterator(map, tmp) {
        scene_destroy((scene_t *)tmp);
    }
    map_destroy(&self->scenes);

    self->current_scene = NULL;


    free(self);
    self = NULL;

    global_poggen = NULL;

}

#endif
