#pragma once
#include <SDL2/SDL_scancode.h>
#include <poglib/basic.h>
#include <poglib/application/window/sdl_window.h>


typedef enum {
  COMMANDINPUTKEY_TYPE_KEYBOARD = 0,
  COMMANDINPUTKEY_TYPE_MOUSE = 1,
  COMMANDINPUTKEY_TYPE_CONTROLLER = 2,    // gamepad button or axis-as-button
  COMMANDINPUTKEY_TYPE_COUNT
} commandinputkey_type;

//NOTE: each binding maps an input source to a command bit
//      .command explicitly sets which bit to toggle in the bitmask (decoupled from array index)
//      this allows OR bindings (e.g., keyboard + controller for the same command)
typedef struct {
    commandinputkey_type type;
    u16                 command;            // which command bit to set when triggered
    union {
        struct {
            SDL_Scancode scancode;
        } sdl_keyboard_key;
        struct {
            sdl_mousebuttontype key;
            sdl_mousewheelstate wheel;
            sdl_mousestate      trigger;
        } sdl_mouse;
        struct {
            SDL_GameControllerButton button;    // checked when axis_as_button.deadzone == 0
            struct {
                SDL_GameControllerAxis axis;
                i16 deadzone;                   // minimum axis value to trigger (>0 enables axis mode)
                bool positive;                  // true = positive direction, false = negative direction
            } axis_as_button;                   // treats analog stick/trigger as a digital button
        } sdl_controller;
    };
} commandinputbinding_t;

#define MAX_COMMAND_TYPE_ALLOWED 255

typedef struct {
    u16                     count;
    commandinputbinding_t   registry[MAX_COMMAND_TYPE_ALLOWED];
} commandregistry_t;


