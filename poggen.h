#pragma once
#include "./util/assetmanager.h"
#include "./poggen/scene.h"

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
#define             poggen_add_scene(PGEN, SCENE_NAME)                          __impl_poggen_add_scene((PGEN), __impl_scene_init(SCENE_NAME))
void                poggen_remove_scene(poggen_t *self, const char *label);
void                poggen_change_scene(poggen_t *self, const char *scene_label);

void                poggen_update(poggen_t *self);
#define             poggen_render(PGEN)                                         (PGEN)->current_scene->__render((PGEN)->current_scene)

void                poggen_destroy(poggen_t *self);


/*----------------------------------------------------------------------------*/

#ifndef IGNORE_POGGEN_IMPLEMENTATION

#define MAX_SCENES_ALLOWED 10



poggen_t * poggen_init(void)
{
    if (!global_window)     eprint("A window is required to run poggen\n");
    if (global_poggen)      eprint("Trying to initialize a second `poggen` in the same instance");

    poggen_t tmp =  {
        .assets         = assetmanager_init(),
        .scenes         = map_init(MAX_SCENES_ALLOWED, scene_t ),
        .current_scene  = NULL,
        .__window       = global_window
    };

    poggen_t *output = (poggen_t *)calloc(1, sizeof(poggen_t ));
    assert(output);

    *output         = tmp;
    global_poggen   = output;

    return output;
}

void __impl_poggen_add_scene(poggen_t *self, const scene_t scene)
{
    assert(self);
    map_t *map = &self->scenes;
    scene_t *tmp = (scene_t *)map_insert(map, scene.label, scene);

    if (!self->current_scene)
        self->current_scene = tmp;

    tmp->__init(tmp);
}


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

void __poggen_update_user_input(poggen_t *self)
{    
    scene_t *current_scene = self->current_scene;
    assert(current_scene);

    window_t *win = self->__window;
    window_update_user_input(win);

    const list_t *actions = &current_scene->__actions;
    list_iterator(actions, i) {

        assert(i); 
        action_t action = *(action_t *)i; 

        switch(win->lastframe.state)
        {
            case SDL_KEYUP:
            case SDL_KEYDOWN:
                if (action.key == win->lastframe.key) {
                    current_scene->__input(action); 
                } 
            break;
        }
    }
}

void poggen_update(poggen_t *self)
{
    assert(self);
    scene_t *current_scene = self->current_scene;
    if (current_scene == NULL) eprint("Current scene is null");

    __poggen_update_user_input(self);
    current_scene->__update(current_scene);

}

void poggen_destroy(poggen_t *self)
{
    assert(self);

    assetmanager_destroy(&self->assets);

    map_t *map = &self->scenes;

    map_iterator(map, tmp) {
        __scene_destroy((scene_t *)tmp);
    }
    map_destroy(&self->scenes);

    self->current_scene = NULL;


    free(self);
    self = NULL;

    global_poggen = NULL;

}

#endif
