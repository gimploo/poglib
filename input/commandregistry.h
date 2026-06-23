#pragma once
#include <SDL2/SDL_scancode.h>
#include <poglib/basic.h>
#include <poglib/application/window/sdl_window.h>


typedef enum {
  COMMANDINPUTKEY_TYPE_KEYBOARD = 0,
  COMMANDINPUTKEY_TYPE_MOUSE = 1,
  COMMANDINPUTKEY_TYPE_CONTROLLER = 2,
  COMMANDINPUTKEY_TYPE_COUNT
} commandinputkey_type;

typedef struct {
    commandinputkey_type type;
    u16                 command;
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
            SDL_GameControllerButton button;
            struct {
                SDL_GameControllerAxis axis;
                i16 deadzone;
                bool positive;
            } axis_as_button;
        } sdl_controller;
    };
} commandinputbinding_t;

#define MAX_COMMAND_TYPE_ALLOWED 255

typedef struct {
    u16                     count;
    commandinputbinding_t   registry[MAX_COMMAND_TYPE_ALLOWED];
} commandregistry_t;


