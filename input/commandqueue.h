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
    const commandregistry_t *commands = &self->registry;

    //printf("-------------------- NEW BATCH --------------------------------\n");
    for (u16 command_type = 0; command_type < commands->count; command_type++) {

        if (self->registry.registry[command_type].type == COMMANDINPUTKEY_TYPE_KEYBOARD) {

            const bool mainkey_held             = global_window->keyboard.is_held[self->registry.registry[command_type].sdl_keyboard_key.main];
            const bool mainkey_justpressed      = global_window->keyboard.just_pressed[self->registry.registry[command_type].sdl_keyboard_key.main];
            const bool mainkey_pressed          = global_window->keyboard.keystate[self->registry.registry[command_type].sdl_keyboard_key.main];

            const bool modifier_not_configured_or_configured_and_pressed  = 
                self->registry.registry[command_type].sdl_keyboard_key.modifier == SDL_SCANCODE_UNKNOWN || 
                global_window->keyboard.keystate[self->registry.registry[command_type].sdl_keyboard_key.modifier];

            if (!modifier_not_configured_or_configured_and_pressed)
                continue;

            switch(self->registry.registry[command_type].sdl_keyboard_key.trigger)
            {
                case COMMANDINPUT_TRIGGER_TYPE_PRESSED:
                    if (mainkey_pressed) self->internal.bitmask |= (1 << command_type);
                break;
                case COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED:
                    if (mainkey_justpressed) self->internal.bitmask |= (1 << command_type);
                break;
                case COMMANDINPUT_TRIGGER_TYPE_HELD:
                    if (mainkey_held) self->internal.bitmask |= (1 << command_type);
                break;
                default: eprint("trigger type not accounted for");
            }

            //printf("KB Tracked %i\n | bitmask %i\n", command_type, self->internal.bitmask);

        } else if (self->registry.registry[command_type].type == COMMANDINPUTKEY_TYPE_MOUSE) {

            const sdl_mousebuttontype key   = self->registry.registry[command_type].sdl_mouse.key;
            const sdl_mousewheelstate wheel = self->registry.registry[command_type].sdl_mouse.wheel;

            const bool found_match = (key != SDL_MOUSEBUTTON_NONE && commandqueue__internal_check_mousebutton_trigger(self->registry.registry[command_type]))
                || (wheel == SDL_MOUSEWHEEL_UP && window_mouse_wheel_is_scroll_up(global_window))
                || (wheel == SDL_MOUSEWHEEL_DOWN && window_mouse_wheel_is_scroll_down(global_window));

            if (!found_match) continue;

            self->internal.bitmask |= (1 << command_type);

            //printf("MS Tracked %i\n", command_type);
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
