#pragma once
#include <poglib/basic.h>
#include "./commandregistry.h"
#include "poglib/application/window/sdl_window.h"
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

#define MAX_ALLOWED_COMMANDS_PER_FRAME 6

typedef u16 command_t;

typedef struct {

    commandregistry_t registry;

    struct {
        u16 bitmask;
    } internal;

} commandqueue_t;


commandqueue_t      commandqueue(const commandregistry_t registry);
void                commandqueue_sync(commandqueue_t * const self);
void                commandqueue_update_registry(commandqueue_t *const self, const commandregistry_t registry);
u16                 commandqueue_get_commands_as_bitmask(const commandqueue_t * const self);
void                commandqueue_flush(commandqueue_t * const self);


#ifndef IGNORE_COMMAND_BUFFER_IMPLEMENTATION

bool commandqueue__internal_check_mousebutton_trigger(const commandinputbinding_t input);

commandqueue_t commandqueue(const commandregistry_t registry) 
{
    return(commandqueue_t) {
        .registry = registry,
        .internal = {
            .bitmask = 0
        }
    };
}


void cq__internal_print_u16_bitmask(uint16_t mask) {
    for (int i = 15; i >= 0; i--) {
        uint16_t bit = (mask >> i) & 1;
        printf("%u", bit);
        if (i % 4 == 0 && i != 0) printf(" "); // Add spacing for readability
    }
    printf("\n");
}

void commandqueue_flush(commandqueue_t *const self)
{
    ASSERT(self);
    self->internal.bitmask = 0;
}

void commandqueue_sync(commandqueue_t * const self)
{
    const u8 *keyboard_buffer = SDL_GetKeyboardState(NULL);
    const commandregistry_t *commands = &self->registry;

    //printf("-------------------- NEW BATCH --------------------------------\n");
    for (u16 i = 0; i < commands->count; i++) {

        const commandinputbinding_t *b = &self->registry.registry[i];

        if (b->type == COMMANDINPUTKEY_TYPE_KEYBOARD && keyboard_buffer[b->sdl_keyboard_key.scancode]) {

            self->internal.bitmask |= (1 << b->command);

            //printf("KB Tracked %i\n", b->command);

        } else if (b->type == COMMANDINPUTKEY_TYPE_MOUSE) {

            const sdl_mousebuttontype key   = b->sdl_mouse.key;
            const sdl_mousewheelstate wheel = b->sdl_mouse.wheel;

            const bool found_match = (key != SDL_MOUSEBUTTON_NONE && commandqueue__internal_check_mousebutton_trigger(self->registry.registry[i]))
                || (wheel == SDL_MOUSEWHEEL_UP && window_mouse_wheel_is_scroll_up(global_window))
                || (wheel == SDL_MOUSEWHEEL_DOWN && window_mouse_wheel_is_scroll_down(global_window));

            if (!found_match) continue;

            self->internal.bitmask |= (1 << b->command);

            //printf("MS Tracked %i\n", b->command);

        //NOTE: controller input (button or axis-as-button with deadzone)
        } else if (b->type == COMMANDINPUTKEY_TYPE_CONTROLLER) {

            bool triggered = false;

            //NOTE: if deadzone is set, treat as axis-as-button binding
            if (b->sdl_controller.axis_as_button.deadzone != 0 && window_controller_is_connected(global_window)) {
                i16 val = window_controller_get_axis_value(global_window, b->sdl_controller.axis_as_button.axis);
                triggered = b->sdl_controller.axis_as_button.positive
                    ? val > b->sdl_controller.axis_as_button.deadzone
                    : val < -b->sdl_controller.axis_as_button.deadzone;
            }

            //NOTE: fall back to regular button check if axis didn't trigger
            if (!triggered && window_controller_is_connected(global_window)) {
                triggered = window_controller_button_is_pressed(global_window, b->sdl_controller.button);
            }

            if (triggered) {
                self->internal.bitmask |= (1 << b->command);
            }

        //NOTE: keyboard AND controller OR'd together in a single binding
        } else if (b->type == COMMANDINPUTKEY_TYPE_KEYBOARD_AND_CONTROLLER) {

            bool triggered = keyboard_buffer[b->sdl_keyboard_and_controller.scancode];

            if (!triggered && window_controller_is_connected(global_window)) {

                if (b->sdl_keyboard_and_controller.axis_as_button.deadzone != 0) {
                    i16 val = window_controller_get_axis_value(global_window, b->sdl_keyboard_and_controller.axis_as_button.axis);
                    triggered = b->sdl_keyboard_and_controller.axis_as_button.positive
                        ? val > b->sdl_keyboard_and_controller.axis_as_button.deadzone
                        : val < -b->sdl_keyboard_and_controller.axis_as_button.deadzone;
                } else {
                    triggered = window_controller_button_is_pressed(global_window, b->sdl_keyboard_and_controller.button);
                }
            }

            if (triggered) {
                self->internal.bitmask |= (1 << b->command);
            }
        }
        //printf("Bitmask: ");cq__internal_print_u16_bitmask(self->internal.bitmask);
    }
    //printf("-------------------- END BATCH --------------------------------\n");
}

bool commandqueue__internal_check_mousebutton_trigger(const commandinputbinding_t input)
{
    switch(input.sdl_mouse.trigger)
    {
        case SDL_MOUSESTATE_DRAG:           return window_mouse_button_is_dragged(global_window, input.sdl_mouse.key);
        case SDL_MOUSESTATE_JUST_PRESSED:   return window_mouse_button_just_pressed(global_window, input.sdl_mouse.key);
        default:                            return window_mouse_button_is_pressed(global_window, input.sdl_mouse.key);
    }
}


u16 commandqueue_get_commands_as_bitmask(const commandqueue_t *const self) {
    ASSERT(self);
    return self->internal.bitmask;
}

void commandqueue_update_registry(commandqueue_t *const self, const commandregistry_t registry)
{
    ASSERT(registry.count);
    self->internal.bitmask = 0;
    self->registry = registry;
}

#endif
