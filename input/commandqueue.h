#pragma once
#include <poglib/basic.h>
#include "./commandregistry.h"
#include "poglib/application/window/sdl_window.h"
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

#define MAX_ALLOWED_COMMANDS_PER_FRAME 6

typedef struct {

    commandregistry_t registry;

    struct {
        u32 bitmask;
    } internal;

} commandqueue_t;


commandqueue_t      commandqueue(const commandregistry_t registry);
void                commandqueue_sync(commandqueue_t * const self);
void                commandqueue_update_registry(commandqueue_t *const self, const commandregistry_t registry);
u32                 commandqueue_get_bitmask(const commandqueue_t * const self);
void                commandqueue_flush(commandqueue_t * const self);


#ifndef IGNORE_COMMAND_BUFFER_IMPLEMENTATION

bool commandqueue__internal_check_mousebutton_trigger(const commandinputbinding_t input);
bool commandqueue__internal_check_joystickbutton_justpressed(const commandinputbinding_t input);
bool commandqueue__internal_check_joystickbutton_pressed(const commandinputbinding_t input);
bool commandqueue__internal_check_joystick_axis_changed_from_rest(const commandinputbinding_t input);

commandqueue_t commandqueue(const commandregistry_t registry) 
{
    return(commandqueue_t) {
        .registry = registry,
        .internal = {
            .bitmask = 0
        }
    };
}


void cq__internal_print_u32_bitmask(u32 mask) {
    for (int i = 31; i >= 0; i--) {
        const u32 bit = (mask >> i) & 1;
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

        const commandinputbinding_t binding = self->registry.registry[command_type];

        if (binding.type & COMMANDINPUTKEY_TYPE_KEYBOARD) {

            const bool mainkey_held             = global_window->keyboard.is_held[binding.sdl_keyboard_key.main];
            const bool mainkey_justpressed      = global_window->keyboard.just_pressed[binding.sdl_keyboard_key.main];
            const bool mainkey_pressed          = global_window->keyboard.keystate[binding.sdl_keyboard_key.main];

            const bool modifier_not_configured_or_configured_and_pressed  = 
                binding.sdl_keyboard_key.modifier == SDL_SCANCODE_UNKNOWN || 
                global_window->keyboard.keystate[binding.sdl_keyboard_key.modifier];

            if (modifier_not_configured_or_configured_and_pressed) {
                switch(binding.sdl_keyboard_key.trigger)
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
            }
            //printf("KB Tracked %i\n | bitmask %i\n", command_type, self->internal.bitmask);
        }

        if (binding.type & COMMANDINPUTKEY_TYPE_MOUSE) {

            const bool modifier_not_configured_or_configured_and_pressed  = 
                binding.sdl_mouse.modifier == SDL_SCANCODE_UNKNOWN || 
                global_window->keyboard.keystate[binding.sdl_mouse.modifier];

            const sdl_mousebuttontype key   = binding.sdl_mouse.key;
            const sdl_mousewheelstate wheel = binding.sdl_mouse.wheel;

            const bool found_match = (modifier_not_configured_or_configured_and_pressed)
                && ((key != SDL_MOUSEBUTTON_NONE && commandqueue__internal_check_mousebutton_trigger(binding))
                    || (wheel == SDL_MOUSEWHEEL_UP && window_mouse_wheel_is_scroll_up(global_window))
                    || (wheel == SDL_MOUSEWHEEL_DOWN && window_mouse_wheel_is_scroll_down(global_window)));

            if (found_match) self->internal.bitmask |= (1 << command_type);

            //printf("MS Tracked %i\n", command_type);
        }

        if (binding.type & COMMANDINPUTKEY_TYPE_JOYSTICK) {

            if (binding.sdl_gamecontroller.type == COMMANDINPUT_CONTROLLER_BUTTON) {
                if (binding.sdl_gamecontroller.button.trigger == COMMANDINPUT_TRIGGER_TYPE_PRESSED && commandqueue__internal_check_joystickbutton_pressed(binding))
                    self->internal.bitmask |= (1 << command_type);
                else if (binding.sdl_gamecontroller.button.trigger == COMMANDINPUT_TRIGGER_TYPE_JUSTPRESSED && commandqueue__internal_check_joystickbutton_justpressed(binding))
                    self->internal.bitmask |= (1 << command_type);
            }

            if (binding.sdl_gamecontroller.type == COMMANDINPUT_CONTROLLER_AXIS && commandqueue__internal_check_joystick_axis_changed_from_rest(binding)) 
                self->internal.bitmask |= (1 << command_type);
        }
        //printf("Bitmask: ");cq__internal_print_u32_bitmask(self->internal.bitmask);
    }
    //printf("-------------------- END BATCH --------------------------------\n");
}

INTERNAL bool commandqueue__internal_check_mousebutton_trigger(const commandinputbinding_t input)
{
    switch(input.sdl_mouse.trigger)
    {
        case SDL_MOUSESTATE_DRAG:           return window_mouse_button_is_dragged(global_window, input.sdl_mouse.key);
        case SDL_MOUSESTATE_JUST_PRESSED:   return window_mouse_button_just_pressed(global_window, input.sdl_mouse.key);
        default:                            return window_mouse_button_is_pressed(global_window, input.sdl_mouse.key);
    }
}

INTERNAL bool commandqueue__internal_check_joystick_axis_changed_from_rest(const commandinputbinding_t input)
{
    const f32 STICK_THRESHOLD = 4000.f;

    const u8 sign_type = (u8)input.sdl_gamecontroller.axis.sign;

    switch(input.sdl_gamecontroller.axis.data)
    {
        case SDL_CONTROLLER_AXIS_LEFTX:
#if 0
            if (global_window->gamecontroller.stick[0].dir.x != 0)
                printf("%f\n", global_window->gamecontroller.stick[0].dir.x);
#endif
            return (sign_type == COMMANDINPUT_CONTROLLER_AXIS_POSITIVE && global_window->gamecontroller.internal.stick_dirs[0].x > STICK_THRESHOLD)
                || (sign_type == COMMANDINPUT_CONTROLLER_AXIS_NEGATIVE && global_window->gamecontroller.internal.stick_dirs[0].x < -STICK_THRESHOLD);
        break;

        case SDL_CONTROLLER_AXIS_LEFTY:
#if 0
            if (global_window->gamecontroller.stick[0].dir.y != 0)
                printf("%f\n", global_window->gamecontroller.stick[0].dir.y);
#endif
            return (sign_type == COMMANDINPUT_CONTROLLER_AXIS_POSITIVE && global_window->gamecontroller.internal.stick_dirs[0].y > STICK_THRESHOLD)
                || (sign_type == COMMANDINPUT_CONTROLLER_AXIS_NEGATIVE && global_window->gamecontroller.internal.stick_dirs[0].y < -STICK_THRESHOLD);
        break;

        case SDL_CONTROLLER_AXIS_RIGHTX:
#if 0
            if (global_window->gamecontroller.stick[1].dir.x != 0)
                printf("%f\n", global_window->gamecontroller.stick[1].dir.x);
#endif
            return (sign_type == COMMANDINPUT_CONTROLLER_AXIS_POSITIVE && global_window->gamecontroller.internal.stick_dirs[1].x > STICK_THRESHOLD)
                || (sign_type == COMMANDINPUT_CONTROLLER_AXIS_NEGATIVE && global_window->gamecontroller.internal.stick_dirs[1].x < -STICK_THRESHOLD);
        break;

        case SDL_CONTROLLER_AXIS_RIGHTY:
#if 0
            if (global_window->gamecontroller.stick[1].dir.y != 0)
                printf("%f\n", global_window->gamecontroller.stick[1].dir.y);
#endif
            return (sign_type == COMMANDINPUT_CONTROLLER_AXIS_POSITIVE && global_window->gamecontroller.internal.stick_dirs[1].y > STICK_THRESHOLD)
                || (sign_type == COMMANDINPUT_CONTROLLER_AXIS_NEGATIVE && global_window->gamecontroller.internal.stick_dirs[1].y < -STICK_THRESHOLD);
        break;

        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
#if 0
            if (global_window->gamecontroller.trigger.left)
                printf("%f\n", global_window->gamecontroller.trigger.left);
#endif
            return global_window->gamecontroller.trigger.left != 0.f;
        break;

        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
#if 0
            if (global_window->gamecontroller.trigger.right)
                printf("%f\n", global_window->gamecontroller.trigger.right);
#endif
            return global_window->gamecontroller.trigger.right != 0.f;
        break;

        default: eprint("unknown game controller axis registered, check the passed command registry");
    }

    return false;
}


INTERNAL bool commandqueue__internal_check_joystickbutton_pressed(const commandinputbinding_t input)
{
    const SDL_GameControllerButton button = input.sdl_gamecontroller.button.data;
    if ((u8)button >= ARRAY_LEN(global_window->gamecontroller.button.pressed)) return false;
    return global_window->gamecontroller.button.pressed[button];
}

INTERNAL bool commandqueue__internal_check_joystickbutton_justpressed(const commandinputbinding_t input)
{
    const SDL_GameControllerButton button = input.sdl_gamecontroller.button.data;
    if ((u8)button >= ARRAY_LEN(global_window->gamecontroller.button.justpressed)) return false;
    return global_window->gamecontroller.button.justpressed[button];
}


u32 commandqueue_get_bitmask(const commandqueue_t *const self) {
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
