#pragma once
#include "./simple/window.h"
#include "./simple/glrenderer2d.h"
#include "./ecs/entitymanager.h"
#include "./poggen/assetmanager.h"


/*=====================================
            - GAME ENGINE -
=====================================*/


typedef struct poggen_t poggen_t ;




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

    scene_t         *current_scene;

    hashtable_t     scenes;

    assetmanager_t  assets;

};

#endif
