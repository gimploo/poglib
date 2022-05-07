#pragma once
#include <SDL2/SDL.h>
#include "../str.h"

/*
 * This solution, allows me to define player defined actions better as i dont have
 * to worry about the order i define as it is handled by the __is_active flag, 
 * if the flag is true, would mean an action is happening and till its false,
 * a newer action can happen. I could probably nest multiple actions together,
 * like shitf w to sprint and only w as normal walk, need to test it before 
 * concluding
 */


typedef struct action_t action_t ;
#define     action(LABEL, KEY)         __impl_action_init(LABEL, KEY)


#ifndef IGNORE_POGGEN_ACTION_IMPLEMENTATION


struct action_t {

    const char          *label;
    union {
        const SDL_Keycode   key;
    };

};

action_t __impl_action_init(const char *label, const SDL_Keycode key)
{
    return (action_t ) {
        .label = label,
        .key = key
    };
}

#endif
