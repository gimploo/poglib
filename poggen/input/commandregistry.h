#pragma once
#include "SDL_scancode.h"
#include <poglib/basic.h>
#include <poglib/application/window/sdl_window.h>


//INFO:  this requires command actions to be enums as those are the 
//index that maps to the specific sdl provided input state (command key)
//
//
typedef enum {
  COMMANDINPUTKEY_TYPE_KEYBOARD = 0,
  COMMANDINPUTKEY_TYPE_MOUSE = 1,
  COMMANDINPUTKEY_TYPE_COUNT
} commandinputkey_type;

typedef struct {
    commandinputkey_type type;
    union {
        SDL_Scancode sdl_keyboard_key;
        sdl_mousebuttontype sdl_mouse_key;
    };
} commandinputbinding_t;

#define MAX_COMMAND_TYPE_ALLOWED 255

typedef struct {
    u16                 count;
    commandinputbinding_t   registry[MAX_COMMAND_TYPE_ALLOWED];
} commandregistry_t;


