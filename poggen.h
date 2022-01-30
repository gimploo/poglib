#pragma once
#include "./simple/window.h"
#include "./simple/glrenderer2d.h"


/*=====================================
            - GAME ENGINE -
=====================================*/


typedef struct poggen_t poggen_t ;




#ifndef IGNORE_POGGEN_IMPLEMENTATION

typedef struct scene_t scene_t ;

struct scene_t {

    entitymanager_t manager;
    bool            is_paused;
    bool            is_finished;

};

struct poggen_t {

    window_t window;
    glrenderer2d_t renderer2d;

    char *current_scene;

    list_t scenes;

    //TODO: scene handler
    //TODO: asset handler

};

#endif
