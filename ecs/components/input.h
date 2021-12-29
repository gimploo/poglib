#pragma once
#include "../../simple/window.h"



typedef struct c_input_t c_input_t ;



c_input_t       c_input_init(window_t *win);

// MOUSE
#define         c_input_mouse_button_just_pressed(PINPUT)   (PINPUT)->win.mouse_handler.just_pressed
#define         c_input_mouse_button_is_held(PINPUT)        (PINPUT)->win.mouse_handler.is_held

// KEYBOARD
#define         c_input_keyboard_is_key_just_pressed(PINPUT, KEY)   ((PINPUT)->win.keyboard_handler.just_pressed[SDL_GetScancodeFromKey(KEY)] == true)
#define         c_input_keyboard_is_key_held(PINPUT, KEY)           ((PINPUT)->win.keyboard_handler.is_held[SDL_GetScancodeFromKey(KEY)] == true)
#define         c_input_keyboard_is_key_pressed(PINPUT, KEY)        ((PINPUT)->win.keyboard_handler.keystate[SDL_GetScancodeFromKey(KEY)] == true)
#define         c_input_keyboard_is_key_released(PINPUT, KEY)       ((PINPUT)->win.keyboard_handler.keystate[SDL_GetScancodeFromKey(KEY)] == false)





#ifndef IGNORE_C_INPUT_IMPLEMENTATION

struct c_input_t {

    window_t * const win;
};


c_input_t c_input_init(window_t *win)
{
    return (c_input_t ) {
        .win = win
    };
}



#endif 
