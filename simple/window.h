#ifndef _SIMPLE_WINDOW_H_
#define _SIMPLE_WINDOW_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#ifdef __gl_h_
#include <SDL2/SDL_opengl.h>
#else 
#include <SDL2/SDL_render.h>
#define GL_LOG
#endif

#include "../math/la.h"
#include "window/delta_time.h"

#define DEFAULT_BACKGROUND_COLOR (vec4f_t){{ 0.0f, 1.0f, 0.0f, 0.0f}}
#define SDL_FLAGS u32

/*==============================================================================
 // Window library that is wrapper around SDL2 
==============================================================================*/


// Mouse 
typedef struct __mouse_t {

    bool    just_pressed;
    bool    is_held;
    vec2f_t norm_position;
    vec2i_t position;

} __mouse_t;


// Keyboard
typedef struct __keyboard_t {

    bool keystate[SDL_NUM_SCANCODES]; 

    bool just_pressed[SDL_NUM_SCANCODES]; 
    bool is_held[SDL_NUM_SCANCODES]; 

} __keyboard_t;

typedef struct window_t {

    u32             SDL_Window_ID;                  // useful for managing multiple windows
    bool            is_open;
    const char      *title_name;
    u64             width; 
    u64             height;
    SDL_Window      *window_handle;                 // initializes the window 
    __mouse_t       mouse_handler;
    __keyboard_t    keyboard_handler;
    vec4f_t         background_color;


#ifndef __gl_h_                                     // initializes the renderer
    SDL_Surface    *surface_handle;
#else 
    SDL_GLContext   gl_context;
#endif 

    game_loop_time_t    time;

    //TODO: this approach was to have a debug window, but at the moment can only hold one sub window at a time. In the future lets implement a list of sub windows 
    // Sub window logic
    struct window_t     *sub_window_handle;             // Holds sub_window address
    bool                is_sub_window_active;

} window_t;

typedef void (*render_func) (void*);

/*----------------------------------------------------------------------
 // Declarations
----------------------------------------------------------------------*/

// Initializing +++++++++++++++++++++++++++++++++++++++++++++++++++++++

window_t        window_init(const char *title, size_t width, size_t height, SDL_FLAGS flags);
window_t *      window_sub_window_init(window_t *parent, const char *title_name, size_t width, size_t height, SDL_FLAGS flags);

// Helper ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void            window_set_background(window_t *window, vec4f_t color);
void            window_update_title(window_t *window, const char *title_name);

// Input ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//NOTE:(macro)  window_mouse_get_norm_position(window_t *window) -> vec2f_t
//NOTE:(macro)  window_mouse_just_pressed(window_t *window) -> bool
//NOTE:(macro)  window_mouse_is_held(window_t *window) -> bool

//NOTE:(macro)  window_keyboard_is_key_just_pressed(window_t *window, SDL_KeyCode key)  -> bool
//NOTE:(macro)  window_keyboard_is_key_held(window_t *window, SDL_KeyCode key)       -> bool
//NOTE:(macro)  window_keyboard_is_key_pressed(window_t *window, SDL_KeyCode key)       -> bool
//NOTE:(macro)  window_keyboard_is_key_released(window_t *window, SDL_KeyCode key)      -> bool

//NOTE:(macro)  window_grab_dt(window_t *window) -> f64
//NOTE:(macro)  window_grab_fps(window_t *window) -> f64

// Render ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
//NOTE:(macro)  window_game_while_loop(window_t *window) // this is the normal while(window.is_open) loop but with the added dt and fps calculation
//NOTE:(macro)  window_gl_render_begin(window_t *window)
//NOTE:(macro)  window_gl_render_end(window_t *window)
//NOTE:(macro)  window_sub_window_gl_render_begin(window_t *sub_window)
//NOTE:(macro)  window_sub_window_gl_render_end(window_t *sub_window)

void            window_render_stuff(window_t *window, render_func stuff, void * arg);
void            window_sub_window_render_stuff(window_t *sub_window, render_func stuff, void *arg);

// Cleanup ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void            window_destroy(window_t *window);
void            window_sub_window_destroy(window_t *sub_window);


/*-------------------------------------------------------------------------
 // Implementations
-------------------------------------------------------------------------*/

#define window_mouse_get_norm_position(pwindow) (pwindow)->mouse_handler.norm_position
#define window_mouse_just_pressed(pwindow)      (pwindow)->mouse_handler.just_pressed
#define window_mouse_is_held(pwindow)           (pwindow)->mouse_handler.is_held

#define window_game_while_loop(pwindow)   while((pwindow)->is_open && game_loop_time_calculate(&(pwindow)->time))

// FIXME: This function doesnt work
#define window_cap_fps(pwindow)     SDL_Delay(floor(16.666f - (pwindow)->time.elapsed_in_ms))

#define window_grab_dt(pwindow)      (pwindow)->time.dt_value
#define window_grab_fps(pwindow)     (pwindow)->time.fps_value

#define window_gl_render_begin(pwindow) {\
    __window_update_user_input(pwindow);\
    glClearColor(\
            (pwindow)->background_color.cmp[0],\
            (pwindow)->background_color.cmp[1],\
            (pwindow)->background_color.cmp[2],\
            (pwindow)->background_color.cmp[3]\
    );\
    glClear(GL_COLOR_BUFFER_BIT);\
}

#define window_gl_render_end(pwindow) SDL_GL_SwapWindow((pwindow)->window_handle)

#define window_sub_window_gl_render_begin(pwindow) {\
    SDL_GL_MakeCurrent((pwindow)->window_handle, (pwindow)->gl_context);\
    glClearColor(\
            (pwindow)->background_color.cmp[0],\
            (pwindow)->background_color.cmp[1],\
            (pwindow)->background_color.cmp[2],\
            (pwindow)->background_color.cmp[3]\
    );\
    glClear(GL_COLOR_BUFFER_BIT);\
}

#define window_sub_window_gl_render_end(pwindow) SDL_GL_SwapWindow((pwindow)->window_handle)

#define window_keyboard_is_key_just_pressed(pwindow, KEY)   ((pwindow)->keyboard_handler.just_pressed[SDL_GetScancodeFromKey(KEY)] == true)
#define window_keyboard_is_key_held(pwindow, KEY)           ((pwindow)->keyboard_handler.is_held[SDL_GetScancodeFromKey(KEY)] == true)
#define window_keyboard_is_key_pressed(pwindow, KEY)        ((pwindow)->keyboard_handler.key_state[SDL_GetScancodeFromKey(KEY)] == true)
#define window_keyboard_is_key_released(pwindow, KEY)       ((pwindow)->keyboard_handler.key_state[SDL_GetScancodeFromKey(KEY)] == false)

void window_update_title(window_t *window, const char *title_name)
{
    if (window == NULL) eprint("window argument is null");

    window->title_name = title_name;
}

void window_set_background(window_t *window, vec4f_t color) 
{
    if(window == NULL) eprint("window argument is null");

    window->background_color = color;
}


INTERNAL void __mouse_update_position(window_t *window)
{
    int x, y;
    vec2f_t pos;

    // This function dosent produce an error -> Returns a 32-bit button bitmask of the current button state.
    SDL_GetMouseState(&x, &y);

    //SDL_Log("Window (%s) Mouse pos := (%d, %d)\n", window->title_name, x, y);

    f32 normalizedX = -1.0 + 2.0 *  (f32) x / window->width;
    f32 normalizedY = (1.0 - 2.0 * (f32) y / window->height);
    
    pos.cmp[X] = normalizedX;
    pos.cmp[Y] = normalizedY;
    
    SDL_Log("Mouse pos := (%f, %f)\n", normalizedX, normalizedY);
    
    window->mouse_handler.position = (vec2i_t){x, y};
    window->mouse_handler.norm_position = pos;

}


INTERNAL void  __mouse_update(window_t *window)
{
    __mouse_update_position(window);
}

INTERNAL __mouse_t __mouse_init(window_t *window)
{
    __mouse_t mouse = {
        .just_pressed = false,
        .is_held = false,
    };
    __mouse_update_position(window);
}

INTERNAL window_t __sub_window_init(const char *title_name, size_t width, size_t height, SDL_FLAGS flags) 
{   
    window_t output         = {0};
    output.title_name       = title_name;
    output.is_open          = true;
    output.width            = width;
    output.height           = height;
    output.background_color = DEFAULT_BACKGROUND_COLOR;
    output.mouse_handler    = __mouse_init(&output);
    output.time             = game_loop_time_init();

    output.keyboard_handler = (__keyboard_t ) {
        .keystate       = {false},
        .just_pressed   = {false},
        .is_held        = {false}
    };

    output.sub_window_handle = NULL;
    output.is_sub_window_active = false;

    SDL_FLAGS WinFlags      = 0;

#ifdef __gl_h_

    WinFlags = SDL_WINDOW_OPENGL;

#endif 

    if (SDL_Init(flags) == -1) eprint("SDL Error: %s\n", SDL_GetError());

    output.window_handle = SDL_CreateWindow(output.title_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, WinFlags);

    if (!output.window_handle) eprint("SDL Error: %s\n", SDL_GetError());

    if (!SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1"))
        eprint("SDL Error: %s", SDL_GetError());


#ifdef __gl_h_
    glewExperimental = true; // if using GLEW version 1.13 or earlier
    output.gl_context = SDL_GL_CreateContext(output.window_handle);
    if (!output.gl_context) eprint("SDL Error: %s\n", SDL_GetError());

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) eprint("GLEW Error: %s\n", glewGetErrorString(glewError));

    printf("[OUTPUT] Using OpenGL render\n");

#else 
    output.surface_handle = SDL_GetWindowSurface(output.window_handle);
    if (!output.surface_handle) eprint("SDL Error: %s\n", SDL_GetError());

    printf("[OUTPUT] Using standard sdl2 render\n");
#endif

    output.SDL_Window_ID = SDL_GetWindowID(output.window_handle);
    return output;

}

window_t * window_sub_window_init(window_t *parent, const char *title_name, size_t width, size_t height, SDL_FLAGS flags)
{
    if (parent == NULL) eprint("parent window argument is null");

    window_t *sub_window = (window_t *)malloc(sizeof(window_t));
    if (sub_window == NULL) eprint("sub_window malloc failed");

    *sub_window                 = __sub_window_init(title_name, width, height, flags);
    parent->sub_window_handle   = sub_window;

    return sub_window;
}


window_t window_init(const char *title_name, size_t width, size_t height, SDL_FLAGS flags)
{
    window_t output         = {0};
    output.title_name       = title_name;
    output.is_open          = true;
    output.width            = width;
    output.height           = height;
    output.background_color = DEFAULT_BACKGROUND_COLOR;
    output.mouse_handler    = __mouse_init(&output);
    output.time             = game_loop_time_init();

    output.keyboard_handler = (__keyboard_t ) {
        .keystate       = {false},
        .just_pressed   = {false},
        .is_held        = {false}
    };

    output.sub_window_handle = NULL;
    output.is_sub_window_active = false;

    SDL_FLAGS WinFlags      = 0;

#ifdef __gl_h_
    WinFlags = SDL_WINDOW_OPENGL;
    int major_ver, minor_ver;

    if ( !SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major_ver) || 
         !SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor_ver)) 
        eprint("SDL GL Error: %s\n", SDL_GetError());

    if ( !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major_ver) ||
         !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor_ver))

        eprint("SDL GL Error: %s\n", SDL_GetError());

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
                        SDL_GL_CONTEXT_PROFILE_CORE))
        eprint("SDL GL Error: %s\n", SDL_GetError());
#endif 

    if (SDL_Init(flags) == -1) eprint("SDL Error: %s\n", SDL_GetError());

    output.window_handle = SDL_CreateWindow(output.title_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, WinFlags);

    if (!output.window_handle) eprint("SDL Error: %s\n", SDL_GetError());

    if (!SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1"))
        eprint("SDL Error: %s", SDL_GetError());



#ifdef __gl_h_
    glewExperimental = true; // if using GLEW version 1.13 or earlier
    output.gl_context = SDL_GL_CreateContext(output.window_handle);
    if (!output.gl_context) eprint("SDL GL Error: %s\n", SDL_GetError());

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) eprint("GLEW Error: %s\n", glewGetErrorString(glewError));

    printf("[OUTPUT] Using OpenGL render\n");

#else 
    output.surface_handle = SDL_GetWindowSurface(output.window_handle);
    if (!output.surface_handle) eprint("SDL Error: %s\n", SDL_GetError());

    printf("[OUTPUT] Using standard sdl2 render\n");
#endif

    output.SDL_Window_ID = SDL_GetWindowID(output.window_handle);
    return output;

}

INTERNAL void __window_sub_window_handle_window_event(window_t *window, SDL_Event *event)
{
    if (window->SDL_Window_ID == event->window.windowID) {

        switch(event->window.event) 
        {
            case SDL_WINDOWEVENT_CLOSE:
                SDL_Log("Window (%s) close requested", window->title_name);
                window->is_open = false;
            break;

            case SDL_WINDOWEVENT_SHOWN:
                SDL_Log("Window (%s) shown", window->title_name);
            break;

            case SDL_WINDOWEVENT_HIDDEN:
                SDL_Log("Window (%s) hidden", window->title_name);
            break;

            case SDL_WINDOWEVENT_EXPOSED:
                SDL_Log("Window (%s) exposed", window->title_name);
            break;
            
            case SDL_WINDOWEVENT_MOVED:
                SDL_Log("Window (%s) moved to %d,%d", window->title_name, event->window.data1, event->window.data2);
            break;

            case SDL_WINDOWEVENT_RESIZED:
                SDL_Log("Window (%s) resized to %dx%d", window->title_name, event->window.data1, event->window.data2);
            break;

            case SDL_WINDOWEVENT_SIZE_CHANGED:
                SDL_Log("Window (%s) size changed to %dx%d", window->title_name, event->window.data1, event->window.data2);
            break;

            case SDL_WINDOWEVENT_MINIMIZED:
                SDL_Log("Window (%s) minimized", window->title_name);
            break;

            case SDL_WINDOWEVENT_MAXIMIZED:
                SDL_Log("Window (%s) maximized", window->title_name);
            break;

            case SDL_WINDOWEVENT_RESTORED:
                SDL_Log("Window (%s) restored", window->title_name);
            break;

            case SDL_WINDOWEVENT_ENTER:
                SDL_Log("Mouse entered window (%s)", window->title_name);
            break;

            case SDL_WINDOWEVENT_LEAVE:
                SDL_Log("Mouse left window (%s)", window->title_name);
            break;

            case SDL_WINDOWEVENT_FOCUS_GAINED:
                SDL_Log("Window (%s) gained keyboard focus", window->title_name);
            break;

            case SDL_WINDOWEVENT_FOCUS_LOST:
                SDL_Log("Window (%s) lost keyboard focus", window->title_name);
            break;

            case SDL_WINDOWEVENT_TAKE_FOCUS:
                SDL_Log("Window (%s) is offered a focus", window->title_name); 
            break;

            case SDL_WINDOWEVENT_HIT_TEST:
                SDL_Log("Window (%s) has a special hit test", window->title_name);
            break;

            default:
                SDL_Log("Window (%s) got unknown event %d", window->title_name, event->window.event);
            break;
        }

    } else if (window->sub_window_handle->SDL_Window_ID == event->window.windowID) {

        switch(event->window.event) 
        {
            case SDL_WINDOWEVENT_CLOSE:
                SDL_Log("Window (%s) close requested", window->sub_window_handle->title_name);
                window->sub_window_handle->is_open = false;
            break;

            case SDL_WINDOWEVENT_SHOWN:
                SDL_Log("Window (%s) shown", window->sub_window_handle->title_name);
            break;

            case SDL_WINDOWEVENT_HIDDEN:
                SDL_Log("Window (%s) hidden", window->sub_window_handle->title_name);
            break;

            case SDL_WINDOWEVENT_EXPOSED:
                SDL_Log("Window (%s) exposed", window->sub_window_handle->title_name);
            break;
            
            case SDL_WINDOWEVENT_MOVED:
                SDL_Log("Window (%s) moved to %d,%d", window->sub_window_handle->title_name, event->window.data1, event->window.data2);
            break;

            case SDL_WINDOWEVENT_RESIZED:
                SDL_Log("Window (%s) resized to %dx%d", window->sub_window_handle->title_name, event->window.data1, event->window.data2);
            break;

            case SDL_WINDOWEVENT_SIZE_CHANGED:
                SDL_Log("Window (%s) size changed to %dx%d", window->sub_window_handle->title_name, event->window.data1, event->window.data2);
            break;

            case SDL_WINDOWEVENT_MINIMIZED:
                SDL_Log("Window (%s) minimized", window->sub_window_handle->title_name);
            break;

            case SDL_WINDOWEVENT_MAXIMIZED:
                SDL_Log("Window (%s) maximized", window->sub_window_handle->title_name);
            break;

            case SDL_WINDOWEVENT_RESTORED:
                SDL_Log("Window (%s) restored", window->sub_window_handle->title_name);
            break;

            case SDL_WINDOWEVENT_ENTER:
                SDL_Log("Mouse entered window (%s)", window->sub_window_handle->title_name);
                window->is_sub_window_active = true;
            break;

            case SDL_WINDOWEVENT_LEAVE:
                SDL_Log("Mouse left window (%s)", window->sub_window_handle->title_name);
                window->is_sub_window_active = false;
            break;

            case SDL_WINDOWEVENT_FOCUS_GAINED:
                SDL_Log("Window (%s) gained keyboard focus", window->sub_window_handle->title_name);
                window->is_sub_window_active = true;
            break;

            case SDL_WINDOWEVENT_FOCUS_LOST:
                SDL_Log("Window (%s) lost keyboard focus", window->sub_window_handle->title_name);
                window->is_sub_window_active = false;
            break;

            case SDL_WINDOWEVENT_TAKE_FOCUS:
                SDL_Log("Window (%s) is offered a focus", window->sub_window_handle->title_name); 
                window->is_sub_window_active = true;
            break;

            case SDL_WINDOWEVENT_HIT_TEST:
                SDL_Log("Window (%s) has a special hit test", window->sub_window_handle->title_name);
            break;

            default:
                SDL_Log("Window (%s) got unknown event %d", window->sub_window_handle->title_name, event->window.event);
            break;
        }
    } else eprint("unkown sub window triggered");

}


INTERNAL void __keyboard_update_buffers(window_t *window, SDL_Keycode act, SDL_Scancode key)
{
    switch(act)
    {
        case SDL_KEYDOWN:
            if (window->is_sub_window_active) {
                SDL_Log("Window (%s) KEY_DOWN: %s\n", window->sub_window_handle->title_name, SDL_GetScancodeName(key));

                if (window->sub_window_handle->keyboard_handler.keystate[key] == true) { 

                    SDL_Log("Window (%s) KEY_HELD: %s\n", window->sub_window_handle->title_name, SDL_GetScancodeName(key));
                    window->sub_window_handle->keyboard_handler.is_held[key]        = true;
                    window->sub_window_handle->keyboard_handler.just_pressed[key]   = false;

                } else if (window->sub_window_handle->keyboard_handler.keystate[key] == false) {

                    SDL_Log("Window (%s) KEY_JUST_PRESSED: %s\n", window->sub_window_handle->title_name, SDL_GetScancodeName(key));
                    window->sub_window_handle->keyboard_handler.just_pressed[key]   = true;
                    window->sub_window_handle->keyboard_handler.is_held[key]        = false;

                }

                window->sub_window_handle->keyboard_handler.keystate[key] = true;

            } else {

                SDL_Log("Window (%s) KEY_DOWN: %s\n", window->title_name, SDL_GetScancodeName(key));
                if (window->keyboard_handler.keystate[key] == true) { 

                    SDL_Log("Window (%s) KEY_HELD: %s\n", window->title_name, SDL_GetScancodeName(key));
                    window->keyboard_handler.is_held[key]        = true;
                    window->keyboard_handler.just_pressed[key]   = false;

                } else if (window->keyboard_handler.keystate[key] == false) {

                    SDL_Log("Window (%s) KEY_JUST_PRESSED: %s\n", window->title_name, SDL_GetScancodeName(key));
                    window->keyboard_handler.just_pressed[key]   = true;
                    window->keyboard_handler.is_held[key]        = false;

                }

                window->keyboard_handler.keystate[key] = true;

            }
        break;

        case SDL_KEYUP:
            if (window->is_sub_window_active) {
                SDL_Log("Window (%s) KEY_UP: %s\n", window->sub_window_handle->title_name, SDL_GetScancodeName(key));
                window->sub_window_handle->keyboard_handler.is_held[key]        = false;
                window->sub_window_handle->keyboard_handler.just_pressed[key]   = false;
                window->sub_window_handle->keyboard_handler.keystate[key]       = false;

            } else {
                SDL_Log("Window (%s) KEY_UP: %s\n", window->title_name, SDL_GetScancodeName(key));
                window->keyboard_handler.is_held[key]        = false;
                window->keyboard_handler.just_pressed[key]   = false;
                window->keyboard_handler.keystate[key]       = false;
            }
        break;
    }
}

INTERNAL void __window_update_user_input(window_t *window)
{
    SDL_Event event;
    while(SDL_PollEvent(&event) > 0) 
    {
        switch (event.type) 
        {
            case SDL_WINDOWEVENT:
                // This event is triggered if more than one window is initialized
                __window_sub_window_handle_window_event(window, &event);
            break;

            case SDL_QUIT:
                window->is_open = false;
            break;

            case SDL_MOUSEMOTION:
                __mouse_update(window);
                if(window->mouse_handler.just_pressed == true) {
                    SDL_Log("Window (%s) MOUSE_HELD\n", window->title_name);
                    window->mouse_handler.just_pressed = false;
                    window->mouse_handler.is_held = true;
                }
            break;

            case SDL_MOUSEBUTTONDOWN:
                __mouse_update(window);
                SDL_Log("Window (%s) MOUSE_JUST_DOWN\n", window->title_name);
                window->mouse_handler.just_pressed  = true;
                window->mouse_handler.is_held       = false;
            break;

            case SDL_MOUSEBUTTONUP:
                SDL_Log("Window (%s) MOUSE_UP\n", window->title_name);
                __mouse_update(window);
                window->mouse_handler.just_pressed  = false;
                window->mouse_handler.is_held       = false;
            break;

            case SDL_KEYDOWN:
                __keyboard_update_buffers(window, SDL_KEYDOWN, event.key.keysym.scancode);
            break;

            case SDL_KEYUP:
                __keyboard_update_buffers(window, SDL_KEYUP, event.key.keysym.scancode);
            break;

            //default:
                //SDL_ShowSimpleMessageBox(0, "ERROR", "Key not accounted for", window->window_handle);
                //window->is_open = false;
        }
    }
}


void window_sub_window_render_stuff(window_t *sub_window, render_func stuff, void *arg)
{
#ifdef __gl_h_
        SDL_GL_MakeCurrent(sub_window->window_handle, sub_window->gl_context);
        glClearColor(
                sub_window->background_color.cmp[0], 
                sub_window->background_color.cmp[1],
                sub_window->background_color.cmp[2],
                sub_window->background_color.cmp[3]
        );
        glClear(GL_COLOR_BUFFER_BIT);
#else 
        u8 color[3] = {
            (u8) denormalize(sub_window->background_color.cmp[X], 0, 255),
            (u8) denormalize(sub_window->background_color.cmp[Y], 0, 255),
            (u8) denormalize(sub_window->background_color.cmp[Z], 0, 255)
        };

        SDL_FillRect(
                sub_window->surface_handle, 
                NULL, 
                SDL_MapRGB(
                    sub_window->surface_handle->format, 
                    color[0], color[1], color[2])
                );
#endif 

        stuff(arg);

#ifdef __gl_h_
        SDL_GL_SwapWindow(sub_window->window_handle);
#else
        SDL_UpdateWindowSurface(sub_window->window_handle);
#endif
    
}

void window_render_stuff(window_t *window, render_func render, void *arg)
{
    if (window == NULL) eprint("window argument is null");

    __window_update_user_input(window);

#ifdef __gl_h_
    SDL_GL_MakeCurrent(window->window_handle, window->gl_context);
    glClearColor(
            window->background_color.cmp[0], 
            window->background_color.cmp[1],
            window->background_color.cmp[2],
            window->background_color.cmp[3]
    );
    glClear(GL_COLOR_BUFFER_BIT);
#else 
    u8 color[3] = {
        (u8) denormalize(window->background_color.cmp[X], 0, 255),
        (u8) denormalize(window->background_color.cmp[Y], 0, 255),
        (u8) denormalize(window->background_color.cmp[Z], 0, 255)
    };

    SDL_FillRect(
            window->surface_handle, 
            NULL, 
            SDL_MapRGB(
                window->surface_handle->format, 
                color[0], color[1], color[2])
            );
#endif 

    render(arg);



#ifdef __gl_h_
    SDL_GL_SwapWindow(window->window_handle);
#else
    SDL_UpdateWindowSurface(window->window_handle);
#endif

}

void window_sub_window_destroy(window_t *sub_window)
{
    assert(sub_window);

#   ifdef __gl_h_
        SDL_GL_DeleteContext(sub_window->gl_context);
#   else 
        SDL_FreeSurface(sub_window->surface_handle);
        sub_window->surface_handle = NULL;
#   endif 

    SDL_DestroyWindow(sub_window->window_handle);
    sub_window->window_handle = NULL;

    sub_window = NULL;
}

void window_destroy(window_t *window)
{
    if (window == NULL) eprint("window argument is null");

#ifdef __gl_h_
    SDL_GL_DeleteContext(window->gl_context);
#else 
    SDL_FreeSurface(window->surface_handle);
    window->surface_handle = NULL;
#endif 

    if (window->sub_window_handle != NULL) {
        window_sub_window_destroy(window->sub_window_handle);
        window->sub_window_handle = NULL;
        window->is_sub_window_active = false;
    }

    free(window->sub_window_handle);
    SDL_DestroyWindow(window->window_handle);
    SDL_Quit();

    window->window_handle = NULL;
    window = NULL;
    
}



#endif // __SIMPLE_WINDOW_H_
