#pragma once
#include <SDL2/SDL.h>
#include "../str.h"

//NOTE: Not used at the moment, felt as if i am overenginerring a already simple
//problem


typedef struct action_t action_t ;
#define     action(LABEL, KEY)         __impl_action_init(LABEL, (SDLK_##KEY))



#ifndef IGNORE_POGGEN_ACTION_IMPLEMENTATION


struct action_t {

    const char          *label;
    const SDL_Keycode   key;

};

action_t __impl_action_init(const char *label, const SDL_Keycode key)
{
    return (action_t ) {
        .label = label,
        .key = key
    };
}

#endif
