#pragma once
#include <SDL2/SDL_scancode.h>
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

typedef enum {
    COMMANDINPUT_TRIGGER_TYPE_PRESSED       = 0,
    COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED   = 1,
    COMMANDINPUT_TRIGGER_TYPE_HELD          = 2,
} commandinput_trigger_type;

typedef struct {
    commandinputkey_type type;
    union {
        struct {
            SDL_Scancode                main;
            SDL_Scancode                modifier;
            commandinput_trigger_type   trigger;
        } sdl_keyboard_key;
        struct {
            sdl_mousebuttontype key;
            sdl_mousewheelstate wheel;
            sdl_mousestate      trigger;
        } sdl_mouse;
    };
} commandinputbinding_t;

#define MAX_COMMAND_TYPE_ALLOWED 255

typedef struct {
    u16                     count;
    commandinputbinding_t   registry[MAX_COMMAND_TYPE_ALLOWED];
} commandregistry_t;


