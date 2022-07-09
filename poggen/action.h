#pragma once
#include <SDL2/SDL.h>
#include <poglib/application/window.h>

typedef struct action_t {

    char                label[16];
    union {
        SDL_Keycode     key;
    } ;

} action_t ;

#define     action(LABEL, KEY)         __impl_action_init(LABEL, KEY)


#ifndef IGNORE_POGGEN_ACTION_IMPLEMENTATION


action_t __impl_action_init(const char *label, const SDL_Keycode key)
{
    action_t o = {0};
    memcpy(o.label, label, ARRAY_LEN(o.label) - 1);
    o.key = key;
    return o;
}

#endif
