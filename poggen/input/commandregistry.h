#pragma once
#include <poglib/basic.h>
#include <poglib/application/window/sdl_window.h>


//INFO:  this requires command actions to be enums as those are the 
//index that maps to the specific sdl provided input state (command key)

typedef union {
    SDL_KeyCode sdl_keyboard_key;
    sdl_mousebuttontype sdl_mouse_key;
} commandinputkey_t;

#define MAX_COMMAND_TYPE_ALLOWED sizeof(u8)

typedef struct {
    u16                 count;
    commandinputkey_t   registry[MAX_COMMAND_TYPE_ALLOWED];
} commandregistry_t;


