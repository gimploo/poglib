#pragma once
#include <SDL2/SDL_scancode.h>
#include <poglib/basic.h>
#include <poglib/application/window/sdl_window.h>


//INFO:  each registry entry maps to a command bit (via .command field).
//       The type selects which input source(s) to check:
//       KEYBOARD/MOUSE/CONTROLLER for single source,
//       KEYBOARD_AND_CONTROLLER for OR-ing keyboard + controller inputs.
typedef enum {
  COMMANDINPUTKEY_TYPE_KEYBOARD = 0,
  COMMANDINPUTKEY_TYPE_MOUSE = 1,
  COMMANDINPUTKEY_TYPE_CONTROLLER = 2,              // gamepad button or axis-as-button
  COMMANDINPUTKEY_TYPE_KEYBOARD_AND_CONTROLLER = 3, // keyboard OR controller triggers the command
  COMMANDINPUTKEY_TYPE_COUNT
} commandinputkey_type;

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
        struct {
            SDL_Scancode scancode;              // keyboard key
            SDL_GameControllerButton button;    // controller button (checked when deadzone == 0)
            struct {
                SDL_GameControllerAxis axis;
                i16 deadzone;
                bool positive;
            } axis_as_button;                   // controller axis (checked when deadzone > 0)
        } sdl_keyboard_and_controller;          // checked for BOTH keyboard AND controller
    };
} commandinputbinding_t;

#define MAX_COMMAND_TYPE_ALLOWED 255

typedef struct {
    u16                     count;
    commandinputbinding_t   registry[MAX_COMMAND_TYPE_ALLOWED];
} commandregistry_t;


