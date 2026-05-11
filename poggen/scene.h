#pragma once
#include "poglib/basic/arena.h"
#include "poglib/poggen/input/commandqueue.h"
#include "poglib/poggen/input/commandregistry.h"

/*============================================================================
                            - SCENE -
============================================================================*/

#define MAX_ACTIONS_ALLOWED_PER_SCENE 64

typedef struct scene_t {

    str_t                label;
    arena_t              arena;
    void                 *content;
    commandqueue_t       commandqueue;
    bool                 __is_paused;
    bool                 __is_over;
    void                 (*__init)(struct scene_t *);
    void                 (*__update)(struct scene_t *, const f32 dt);
    void                 (*__input)(struct scene_t *, const f32 dt);
    void                 (*__render)(struct scene_t *, const f32 dt);
    void                 (*__destroy)(struct scene_t *);

} scene_t ;


void * scene_alloc_content(scene_t * const self, const u64 content_size);

#define             scene_get_type(PSCENE)                                     (PSCENE)->__enum_id
#define             scene_get_engine(...)                                      global_poggen
#define             scene_alloc_content(PSCENE, CONTENT_TYPE)\
                    (CONTENT_TYPE *)__impl_scene_alloc_content((PSCENE), sizeof(CONTENT_TYPE))
void                scene_register_input_bindings(scene_t * const self, const commandregistry_t registry);



/*-----------------------------------------------------------------------------*/

#ifndef IGNORE_POGGEN_SCENE_IMPLEMENTATION

void * __impl_scene_alloc_content(scene_t * const self, const u64 content_size) {
    self->content = arena_reserve(&self->arena, content_size);
    return self->content;
}


#define __impl_scene_init(SCENE_NAME)\
    (scene_t ){\
        .label          = str(#SCENE_NAME),\
        .arena          = arena_init(NULL, 5 * MB),\
        .content        = NULL,\
        .__is_paused    = false,\
        .__is_over      = false,\
        .__init         = SCENE_NAME##_init,\
        .__update       = SCENE_NAME##_update,\
        .__input        = SCENE_NAME##_input,\
        .__render       = SCENE_NAME##_render,\
        .__destroy      = SCENE_NAME##_destroy,\
   }


void scene__internal_input_callback(scene_t * const self, const f32 dt) {
    commandqueue_sync_input(&self->commandqueue);
    self->__input(self, dt);
}

void __scene_destroy(scene_t *scene) {
    assert(scene);

    scene->__destroy(scene);

    arena_destroy(&scene->arena)    ;

    scene->label = (str_t){0};
    scene->__init  = NULL;
    scene->__update = NULL;
    scene->__render = NULL;
    scene->__destroy = NULL;
    scene->__input = NULL;
}

void scene_register_input_bindings(scene_t * const self, const commandregistry_t registry) {
    self->commandqueue = commandqueue_init(&self->arena, registry);
}

#endif
