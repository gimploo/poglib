#pragma once
#include "./simple/window.h"
#include "./simple/glrenderer2d.h"
#include "./ecs/entitymanager.h"
#include "./poggen/assetmanager.h"


/*=====================================
            - GAME ENGINE -
=====================================*/


typedef struct poggen_t poggen_t ;

poggen_t        poggen_init(void);
void            poggen_destroy(poggen_t *self);



#ifndef IGNORE_POGGEN_IMPLEMENTATION

typedef struct scene_t scene_t ;

struct scene_t {

    poggen_t        *game_engine;
    entitymanager_t manager;

    //NOTE: apparantly this is a better way to handle user input
    //hashtable_t     action_map; 
    
    bool            is_paused;
    bool            is_over;

    void (*update) (struct scene_t );
    void (*render) (struct scene_t );

};

struct poggen_t {

    window_t        window;
    glrenderer2d_t  renderer2d;
    assetmanager_t  assets;
    hashtable_t     scenes;
    scene_t         *current_scene;
};


poggen_t poggen_init(void)
{
    return (poggen_t ) {
        .window = window_init("Poggen", 800, 700, SDL_INIT_VIDEO | SDL_INIT_AUDIO),
        .renderer2d = glrenderer2d_init(NULL, NULL),
        .assets = assetmanager_init(),
        .scenes = hashtable_init(8, scene_t ),
        .current_scene = NULL,
    };
}


void poggen_destroy(poggen_t *self)
{
    assert(self);

    window_destroy(&self->window);
    glrenderer2d_destroy(&self->renderer2d);
    assetmanager_destroy(&self->assets);
    hashtable_destroy(&self->scenes);
    self->current_scene = NULL;

}

#endif
