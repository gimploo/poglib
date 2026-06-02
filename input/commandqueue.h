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


commandqueue_t      commandqueue(arena_t * const arena, const commandregistry_t registry);
void                commandqueue_sync(commandqueue_t * const self);
u16                 commandqueue_get_commands_as_bitmask(const commandqueue_t * const self);
void                commandqueue_flush(commandqueue_t * const self);


#ifndef IGNORE_COMMAND_BUFFER_IMPLEMENTATION

bool commandqueue__internal_check_mousebutton_trigger(const commandinputbinding_t input);

commandqueue_t commandqueue(arena_t * const arena, const commandregistry_t registry) {
    ASSERT(arena);
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

void commandqueue_flush(commandqueue_t * const self)
{
    ASSERT(self);
    self->internal.bitmask = 0;
}

void commandqueue_sync(commandqueue_t * const self)
{
    const u8 *keyboard_buffer = SDL_GetKeyboardState(NULL);
    const commandregistry_t *commands = &self->registry;

    //self->internal.bitmask = 0;
    //queue_clear(&self->commands);

    //printf("-------------------- NEW BATCH --------------------------------\n");
    for (u16 command_type = 0; command_type < commands->count; command_type++) {

        if (self->registry.registry[command_type].type == COMMANDINPUTKEY_TYPE_KEYBOARD && keyboard_buffer[self->registry.registry[command_type].sdl_keyboard_key]) {

            self->internal.bitmask |= (1 << command_type);

            //printf("KB Tracked %i\n", command_type);

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
        case SDL_MOUSESTATE_DRAG:   return window_mouse_button_is_dragged(global_window, input.sdl_mouse.key);
        default:                    return window_mouse_button_is_pressed(global_window, input.sdl_mouse.key);
    }
}


u16 commandqueue_get_commands_as_bitmask(const commandqueue_t * const self) {
    ASSERT(self);
    return self->internal.bitmask;
}

#endif
