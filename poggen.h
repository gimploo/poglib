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

#define         poggen_add_scene(PGEN, ENUM_SCENE_TYPE)            __impl_poggen_add_scene((PGEN), (ENUM_SCENE_TYPE), (#ENUM_SCENE_TYPE))

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
        .assets  = assetmanager_init(),
        .scenes         = map_init(MAX_SCENES_ALLOWED, scene_t ),
        .current_scene  = NULL,
        .renderer2d     = s_renderer2d_init(),
    };
}


void __impl_poggen_add_scene(poggen_t *self, const u32 enum_id, const char *label)
{
    assert(self);
    assert(label);

    scene_t tmp = scene_init(enum_id, label);
    if (!self->current_scene)
        self->current_scene = map_insert(&self->scenes, label, tmp);
    else
        map_insert(&self->scenes, label, tmp);
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
