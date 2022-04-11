#pragma once
#include "./ecs/entitymanager.h"
#include "./poggen/assetmanager.h"
#include "./poggen/scene.h"
#include "./ecs/systems.h"

//TODO: Actions

/*=============================================================================
                             - GAME ENGINE -
==============================================================================*/


typedef struct poggen_t poggen_t ;


poggen_t        poggen_init(void);

#define         poggen_add_scene(PGEN, SCENE_NAME) __impl_poggen_add_scene(PGEN, SCENE_NAME)

void            poggen_remove_scene(poggen_t *self, const char *label);
void            poggen_change_scene(poggen_t *self, const char *scene_label);
#define         poggen_update_scene(PGEN) (PGEN)->current_scene->update((PGEN)->current_scene)
#define         poggen_render_scene(PGEN) (PGEN)->current_scene->render((PGEN)->current_scene)

void            poggen_destroy(poggen_t *self);



#ifndef IGNORE_POGGEN_IMPLEMENTATION

#define MAX_SCENES_ALLOWED 10


struct poggen_t {

    assetmanager_t  assets;
    map_t           scenes;
    scene_t         *current_scene;

    // ecs systems
    s_renderer2d_t  renderer2d;
};

poggen_t poggen_init(void)
{
    return (poggen_t ) {
        .assets         = assetmanager_init(),
        .scenes         = map_init(MAX_SCENES_ALLOWED, scene_t ),
        .current_scene  = NULL,
        .renderer2d     = s_renderer2d_init(),
    };
}


#define __impl_poggen_add_scene(PGEN, SCENE_NAME) do {\
\
    scene_t __TMP##SCENE_NAME = scene_init(#SCENE_NAME);\
    __TMP##SCENE_NAME.init = SCENE_NAME##_init;\
    __TMP##SCENE_NAME.update = SCENE_NAME##_update;\
    __TMP##SCENE_NAME.render = SCENE_NAME##_render;\
    __TMP##SCENE_NAME.destroy = SCENE_NAME##_destroy;\
    (__TMP##SCENE_NAME).poggen = (PGEN);\
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

    printf("SCENE UPDATED FROM (%s) TO (%s)\n", self->current_scene->label, scene->label);
    self->current_scene = scene;
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

    s_renderer2d_destroy(&self->renderer2d);

}

#endif
