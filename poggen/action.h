#pragma once
#include <SDL2/SDL.h>
#include <poglib/application/window/sdl_window.h>

//THIS IS A MISTAKE 

typedef enum actiontype {

    ACTION_TYPE_JUSTPRESSED,
    ACTION_TYPE_PRESSED,
    ACTION_TYPE_RELEASED,
    ACTION_TYPE_HELD,
    ACTION_TYPE_NONE,

} actiontype;


typedef struct action_t {

    char                label[16];
    actiontype          type;
    union {
        SDL_Keycode     key;
        sdl_mousestate  mouse;
    };

} action_t ;

#define     action(LABEL, TYPE, KEY)         __impl_action_init((LABEL), TYPE, KEY)


#ifndef IGNORE_POGGEN_ACTION_IMPLEMENTATION


action_t __impl_action_init(const char *label, actiontype type, const SDL_Keycode key)
{
    assert(strlen(label) < 16);

    action_t o = {0};
    memcpy(o.label, label, sizeof(o.label));
    o.key = key;
    o.type = type;
    return o;
}

#endif
