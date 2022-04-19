#pragma once
#include "../basic.h"
#include "../math/la.h"
#include <SDL2/SDL.h>
#ifdef __gl_h_
    #include <SDL2/SDL_opengl.h>
#else 
    #include <SDL2/SDL_render.h>
#endif

//TODO: (subwindow) this approach was to have a debug window, but at the moment can only 
//hold one sub window at a time. In the future lets implement a list of 
//sub windows Sub window logic

//TODO: A better way to show sdl logs without it cluttering stdout


#ifndef SDL2_ENABLE_LOG
    #define SDL_Log(fmt, ...)
#endif

/*============================================================================
                    - PLATFORM LAYER (SDL2 Wrapper) -
============================================================================*/

typedef struct window_t {

    bool            is_open;
    const char      *title;
    u64             width; 
    u64             height;
    vec4f_t         background_color;

    struct {

        bool    just_pressed;
        bool    is_held;
        vec2f_t norm_position;
        vec2i_t position;

    } mouse_handler;

    struct {

        bool keystate[SDL_NUM_SCANCODES]; 
        bool just_pressed[SDL_NUM_SCANCODES]; 
        bool is_held[SDL_NUM_SCANCODES];

    } keyboard_handler;

    u32                 SDL_Window_ID;                                            // useful for managing multiple windows
    struct window_t     *__subwindow;                                             // Holds subwindow address
    bool                is_subwindow_active;

    SDL_Window          *__sdl_window;                                            // initializes the window 
    SDL_Event           __sdl_event;
#ifndef __gl_h_                                                                   // initializes the renderer
    SDL_Surface         *__sdl_surface;
#else 
    SDL_GLContext       __glcontext;
#endif 


} window_t ;

global window_t     *global_window = NULL;

window_t *          window_init(const char *title, u64 width, u64 height, const u32 SDL_flags);

void                window_update_user_input(window_t *window);
void                window_set_background(window_t *window, vec4f_t color);
void                window_update_title(window_t *window, const char *title);

#define             window_keyboard_is_key_just_pressed(PWINDOW, KEY)           ((PWINDOW)->keyboard_handler.just_pressed[SDL_GetScancodeFromKey(KEY)] == true)
#define             window_keyboard_is_key_held(PWINDOW, KEY)                   ((PWINDOW)->keyboard_handler.is_held[SDL_GetScancodeFromKey(KEY)] == true)
#define             window_keyboard_is_key_pressed(PWINDOW, KEY)                ((PWINDOW)->keyboard_handler.keystate[SDL_GetScancodeFromKey(KEY)] == true)
#define             window_keyboard_is_key_released(PWINDOW, KEY)               ((PWINDOW)->keyboard_handler.keystate[SDL_GetScancodeFromKey(KEY)] == false)

#define             window_mouse_get_norm_position(PWINDOW)                     (PWINDOW)->mouse_handler.norm_position
#define             window_mouse_get_position(PWINDOW)                          (PWINDOW)->mouse_handler.position
#define             window_mouse_button_just_pressed(PWINDOW)                   (PWINDOW)->mouse_handler.just_pressed
#define             window_mouse_button_is_held(PWINDOW)                        (PWINDOW)->mouse_handler.is_held
#define             window_mouse_button_is_pressed(PWINDOW)                     (window_mouse_button_just_pressed(PWINDOW) || window_mouse_button_is_held(PWINDOW))

#define             window_gl_render_begin(PWINDOW)                             __impl_window_gl_render_begin(PWINDOW)
#define             window_gl_render_end(PWINDOW)                               SDL_GL_SwapWindow((PWINDOW)->__sdl_window)
                        //(or)
void                window_render_stuff(window_t *window, void (*render)(void *), void *arg);

void                window_destroy(void);

/*=============================================================================
// SUB WINDOW
=============*/

window_t *      window_subwindow_init(window_t *parent, const char *title, u64 width, u64 height, const u32 SDL_flags);
#define         window_subwindow_gl_render_begin(PWINDOW) __impl_window_subwindow_gl_render_begin(PWINDOW)
#define         window_subwindow_gl_render_end(PWINDOW) SDL_GL_SwapWindow((PWINDOW)->__sdl_window)
                        //(or)
void            window_subwindow_render_stuff(window_t *subwindow, void (*stuff)(void *), void *arg);
void            window_subwindow_destroy(window_t *subwindow);


/*----------------------------------------------------------------------------
                            IMPLEMENTATION
----------------------------------------------------------------------------*/

#ifndef IGNORE_WINDOW_IMPLEMENTATION

#define DEFAULT_BACKGROUND_COLOR (vec4f_t ){ 0.0f, 1.0f, 0.0f, 0.0f}

#define __impl_window_subwindow_gl_render_begin(PWINDOW) do {\
\
    SDL_GL_MakeCurrent((PWINDOW)->__sdl_window, (PWINDOW)->__glcontext);\
    GL_CHECK(glClearColor(\
            (PWINDOW)->background_color.cmp[0],\
            (PWINDOW)->background_color.cmp[1],\
            (PWINDOW)->background_color.cmp[2],\
            (PWINDOW)->background_color.cmp[3]\
    ));\
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));\
    GL_CHECK(glEnable(GL_BLEND));\
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));\
    GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));\
\
} while(0)

#define __impl_window_gl_render_begin(PWINDOW) do {\
\
    GL_CHECK(glClearColor(\
            (PWINDOW)->background_color.cmp[0],\
            (PWINDOW)->background_color.cmp[1],\
            (PWINDOW)->background_color.cmp[2],\
            (PWINDOW)->background_color.cmp[3]\
    ));\
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));\
    GL_CHECK(glEnable(GL_BLEND));\
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));\
    GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));\
\
} while(0)

void window_update_title(window_t *window, const char *title)
{
    if (window == NULL) eprint("window argument is null");

    window->title = title;
    SDL_SetWindowTitle(window->__sdl_window, title);
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

    //SDL_Log("Window (%s) Mouse pos := (%d, %d)\n", window->title, x, y);

    f32 normalizedX = -1.0 + 2.0 *  (f32) x / window->width;
    f32 normalizedY = (1.0 - 2.0 * (f32) y / window->height);
    
    pos.cmp[X] = normalizedX;
    pos.cmp[Y] = normalizedY;
    
    //SDL_Log("Mouse pos := (%f, %f)\n", normalizedX, normalizedY);
    
    window->mouse_handler.position = (vec2i_t){x, y};
    window->mouse_handler.norm_position = pos;

}


static inline window_t __subwindow_init(const char *title, u64 width, u64 height, const u32 flags) 
{   
    window_t output         = {0};
    output.title            = title;
    output.is_open          = true;
    output.width            = width;
    output.height           = height;
    output.background_color = DEFAULT_BACKGROUND_COLOR;


    output.__subwindow = NULL;
    output.is_subwindow_active = false;

    u32 WinFlags      = 0;

#ifdef __gl_h_


    WinFlags = SDL_WINDOW_OPENGL;

#endif 

    if (SDL_Init(flags) == -1) eprint("SDL Error: %s\n", SDL_GetError());

    output.__sdl_window = SDL_CreateWindow(output.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, WinFlags);

    if (!output.__sdl_window) eprint("SDL Error: %s\n", SDL_GetError());

    if (!SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1"))
        eprint("SDL Error: %s", SDL_GetError());


#ifdef __gl_h_
    glewExperimental = true; // if using GLEW version 1.13 or earlier
    output.__glcontext = SDL_GL_CreateContext(output.__sdl_window);
    if (!output.__glcontext) eprint("SDL Error: %s\n", SDL_GetError());

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) eprint("GLEW Error: %s\n", glewGetErrorString(glewError));

    printf("[OUTPUT] Using OpenGL render\n");

#else 
    output.__sdl_surface = SDL_GetWindowSurface(output.__sdl_window);
    if (!output.__sdl_surface) eprint("SDL Error: %s\n", SDL_GetError());

    printf("[OUTPUT] Using standard sdl2 render\n");
#endif

    output.SDL_Window_ID = SDL_GetWindowID(output.__sdl_window);

    __mouse_update_position(&output);

    return output;

}

window_t * window_subwindow_init(window_t *parent, const char *title, u64 width, u64 height, const u32 flags)
{
    if (parent == NULL) eprint("parent window argument is null");

    window_t *subwindow = (window_t *)malloc(sizeof(window_t));
    if (subwindow == NULL) eprint("subwindow malloc failed");

    *subwindow                 = __subwindow_init(title, width, height, flags);
    parent->__subwindow   = subwindow;

    return subwindow;
}


window_t * window_init(const char *title, u64 width, u64 height, const u32 flags)
{
    if (global_window) eprint("Window already exist, trying to create a second window");

    window_t win         = {0};
    win.title            = title;
    win.is_open          = true;
    win.width            = width;
    win.height           = height;
    win.background_color = DEFAULT_BACKGROUND_COLOR;

    __mouse_update_position(&win);

    win.__subwindow = NULL;
    win.is_subwindow_active = false;

    u32 WinFlags      = 0;

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

    win.__sdl_window = SDL_CreateWindow(win.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, WinFlags);

    if (!win.__sdl_window) eprint("SDL Error: %s\n", SDL_GetError());

    if (!SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1"))
        eprint("SDL Error: %s", SDL_GetError());



#ifdef __gl_h_
    glewExperimental = true; // if using GLEW version 1.13 or earlier
    win.__glcontext = SDL_GL_CreateContext(win.__sdl_window);
    if (!win.__glcontext) eprint("SDL GL Error: %s\n", SDL_GetError());

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) eprint("GLEW Error: %s\n", glewGetErrorString(glewError));

    printf("[OUTPUT] Using OpenGL render\n");

#else 
    win.__sdl_surface = SDL_GetWindowSurface(win.__sdl_window);
    if (!win.__sdl_surface) eprint("SDL Error: %s\n", SDL_GetError());

    printf("[OUTPUT] Using standard sdl2 render\n");
#endif

    win.SDL_Window_ID = SDL_GetWindowID(win.__sdl_window);

    global_window = (window_t *)calloc(1, sizeof(window_t ));
    assert(global_window);

    *global_window = win;

    return global_window;

}

INTERNAL void __window_handle_sdlwindow_event(window_t *window, SDL_Event *event) 
{
    if (window->SDL_Window_ID == event->window.windowID) {

        switch (event->window.event) 
        {
            case SDL_WINDOWEVENT_CLOSE:
                SDL_Log("Window (%s) close requested", window->title);
                window->is_open = false;
            break;

            case SDL_WINDOWEVENT_SHOWN:
                SDL_Log("Window (%s) shown", window->title);
            break;

            case SDL_WINDOWEVENT_HIDDEN:
                SDL_Log("Window (%s) hidden", window->title);
            break;

            case SDL_WINDOWEVENT_EXPOSED:
                SDL_Log("Window (%s) exposed", window->title);
            break;

            case SDL_WINDOWEVENT_MOVED:
                SDL_Log("Window (%s) moved to %d,%d", window->title,
                  event->window.data1, event->window.data2);
              break;

            case SDL_WINDOWEVENT_RESIZED:
                SDL_Log("Window (%s) resized to %dx%d", window->title,
                      event->window.data1, event->window.data2);
            break;

            case SDL_WINDOWEVENT_SIZE_CHANGED:
                SDL_Log("Window (%s) size changed to %dx%d", window->title,
                      event->window.data1, event->window.data2);
            break;

            case SDL_WINDOWEVENT_MINIMIZED:
              SDL_Log("Window (%s) minimized", window->title);
              break;

            case SDL_WINDOWEVENT_MAXIMIZED:
              SDL_Log("Window (%s) maximized", window->title);
              break;

            case SDL_WINDOWEVENT_RESTORED:
              SDL_Log("Window (%s) restored", window->title);
              break;

            case SDL_WINDOWEVENT_ENTER:
              SDL_Log("Mouse entered main window (%s)", window->title);
              break;

            case SDL_WINDOWEVENT_LEAVE:
              SDL_Log("Mouse left main window (%s)", window->title);
              window->mouse_handler.is_held = false;
              window->mouse_handler.just_pressed = false;
              memset(window->keyboard_handler.is_held, false,
                     sizeof(window->keyboard_handler.is_held));
              memset(window->keyboard_handler.just_pressed, false,
                     sizeof(window->keyboard_handler.just_pressed));
              memset(window->keyboard_handler.keystate, false,
                     sizeof(window->keyboard_handler.keystate));
              break;

            case SDL_WINDOWEVENT_FOCUS_GAINED:
              SDL_Log("Window (%s) gained keyboard focus", window->title);
              break;

            case SDL_WINDOWEVENT_FOCUS_LOST:
              SDL_Log("Window (%s) lost keyboard focus", window->title);
              break;

            case SDL_WINDOWEVENT_TAKE_FOCUS:
              SDL_Log("Window (%s) is offered a focus", window->title);
              break;

            case SDL_WINDOWEVENT_HIT_TEST:
              SDL_Log("Window (%s) has a special hit test", window->title);
              break;

            default:
              SDL_Log("Window (%s) got unknown event %d", window->title,
                      event->window.event);
              break;
        }

  } else if (window->__subwindow->SDL_Window_ID == event->window.windowID) {

    switch (event->window.event) {
    case SDL_WINDOWEVENT_CLOSE:
      SDL_Log("Window (%s) close requested",
              window->__subwindow->title);
      window->__subwindow->is_open = false;
      break;

    case SDL_WINDOWEVENT_SHOWN:
      SDL_Log("Window (%s) shown", window->__subwindow->title);
      break;

    case SDL_WINDOWEVENT_HIDDEN:
      SDL_Log("Window (%s) hidden", window->__subwindow->title);
      break;

    case SDL_WINDOWEVENT_EXPOSED:
      SDL_Log("Window (%s) exposed", window->__subwindow->title);
      break;

    case SDL_WINDOWEVENT_MOVED:
      SDL_Log("Window (%s) moved to %d,%d", window->__subwindow->title,
              event->window.data1, event->window.data2);
      break;

    case SDL_WINDOWEVENT_RESIZED:
      SDL_Log("Window (%s) resized to %dx%d",
              window->__subwindow->title, event->window.data1,
              event->window.data2);
      break;

    case SDL_WINDOWEVENT_SIZE_CHANGED:
      SDL_Log("Window (%s) size changed to %dx%d",
              window->__subwindow->title, event->window.data1,
              event->window.data2);
      break;

    case SDL_WINDOWEVENT_MINIMIZED:
      SDL_Log("Window (%s) minimized", window->__subwindow->title);
      break;

    case SDL_WINDOWEVENT_MAXIMIZED:
      SDL_Log("Window (%s) maximized", window->__subwindow->title);
      break;

    case SDL_WINDOWEVENT_RESTORED:
      SDL_Log("Window (%s) restored", window->__subwindow->title);
      break;

    case SDL_WINDOWEVENT_ENTER:
      SDL_Log("Mouse entered window (%s)", window->__subwindow->title);
      window->is_subwindow_active = true;
      break;

    case SDL_WINDOWEVENT_LEAVE:
      SDL_Log("Mouse left window (%s)", window->__subwindow->title);
      window->is_subwindow_active = false;
      break;

    case SDL_WINDOWEVENT_FOCUS_GAINED:
      SDL_Log("Window (%s) gained keyboard focus",
              window->__subwindow->title);
      window->is_subwindow_active = true;
      break;

    case SDL_WINDOWEVENT_FOCUS_LOST:
      SDL_Log("Window (%s) lost keyboard focus",
              window->__subwindow->title);
      window->is_subwindow_active = false;
      break;

    case SDL_WINDOWEVENT_TAKE_FOCUS:
      SDL_Log("Window (%s) is offered a focus",
              window->__subwindow->title);
      window->is_subwindow_active = true;
      break;

    case SDL_WINDOWEVENT_HIT_TEST:
      SDL_Log("Window (%s) has a special hit test",
              window->__subwindow->title);
      break;

    default:
      SDL_Log("Window (%s) got unknown event %d",
              window->__subwindow->title, event->window.event);
      break;
    }
  } else
    eprint("unkown sub window triggered");
}

INTERNAL void __keyboard_update_buffers(window_t *window, SDL_Keycode act, SDL_Scancode key)
{
    switch(act)
    {
        case SDL_KEYDOWN:
            if (window->is_subwindow_active) {

                SDL_Log("Window (%s) KEY_DOWN: %s\n", window->__subwindow->title, SDL_GetScancodeName(key));

                if (window->__subwindow->keyboard_handler.keystate[key] == true) { 

                    SDL_Log("Window (%s) KEY_HELD: %s\n", window->__subwindow->title, SDL_GetScancodeName(key));
                    window->__subwindow->keyboard_handler.is_held[key]        = true;
                    window->__subwindow->keyboard_handler.just_pressed[key]   = false;

                } else if (window->__subwindow->keyboard_handler.keystate[key] == false) {

                    SDL_Log("Window (%s) KEY_JUST_PRESSED: %s\n", window->__subwindow->title, SDL_GetScancodeName(key));
                    window->__subwindow->keyboard_handler.just_pressed[key]   = true;
                    window->__subwindow->keyboard_handler.is_held[key]        = false;

                }

                window->__subwindow->keyboard_handler.keystate[key] = true;

            } else {

                SDL_Log("Window (%s) KEY_DOWN: %s\n", window->title, SDL_GetScancodeName(key));
                if (window->keyboard_handler.keystate[key] == true) { 

                    SDL_Log("Window (%s) KEY_HELD: %s\n", window->title, SDL_GetScancodeName(key));
                    window->keyboard_handler.is_held[key]        = true;
                    window->keyboard_handler.just_pressed[key]   = false;

                } else if (window->keyboard_handler.keystate[key] == false) {

                    SDL_Log("Window (%s) KEY_JUST_PRESSED: %s\n", window->title, SDL_GetScancodeName(key));
                    window->keyboard_handler.just_pressed[key]   = true;
                    window->keyboard_handler.is_held[key]        = false;

                    window->keyboard_handler.keystate[key] = true;
                }

#ifdef __gl_h_

                static bool polymode_flag = false;

                if (window_keyboard_is_key_pressed(window, SDLK_F5)) polymode_flag = !polymode_flag;

                if (polymode_flag) {

                    printf("[!] OPENGL DEBUG MODE (ACTIVATED)\n");
                    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

                } else {
                    printf("[!] OPENGL DEBUG MODE (DISABLED), press `F5` to enable\n");
                    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

                }
#endif

            }
        break;

        case SDL_KEYUP:
            if (window->is_subwindow_active) {
                SDL_Log("Window (%s) KEY_UP: %s\n", window->__subwindow->title, SDL_GetScancodeName(key));
                window->__subwindow->keyboard_handler.is_held[key]        = false;
                window->__subwindow->keyboard_handler.just_pressed[key]   = false;
                window->__subwindow->keyboard_handler.keystate[key]       = false;

            } else {
                SDL_Log("Window (%s) KEY_UP: %s\n", window->title, SDL_GetScancodeName(key));
                window->keyboard_handler.is_held[key]        = false;
                window->keyboard_handler.just_pressed[key]   = false;
                window->keyboard_handler.keystate[key]       = false;
            }
        break;
    }
}


void window_subwindow_render_stuff(window_t *subwindow, void (*stuff)(void *), void *arg)
{
#ifdef __gl_h_
        SDL_GL_MakeCurrent(subwindow->__sdl_window, subwindow->__glcontext);
        glClearColor(
                subwindow->background_color.cmp[0], 
                subwindow->background_color.cmp[1],
                subwindow->background_color.cmp[2],
                subwindow->background_color.cmp[3]
        );
        glClear(GL_COLOR_BUFFER_BIT);
#else 
        u8 color[3] = {
            (u8) denormalize(subwindow->background_color.cmp[X], 0, 255),
            (u8) denormalize(subwindow->background_color.cmp[Y], 0, 255),
            (u8) denormalize(subwindow->background_color.cmp[Z], 0, 255)
        };

        SDL_FillRect(
                subwindow->__sdl_surface, 
                NULL, 
                SDL_MapRGB(
                    subwindow->__sdl_surface->format, 
                    color[0], color[1], color[2])
                );
#endif 

        stuff(arg);

#ifdef __gl_h_
        SDL_GL_SwapWindow(subwindow->__sdl_window);
#else
        SDL_UpdateWindowSurface(subwindow->__sdl_window);
#endif
    
}

void window_update_user_input(window_t *window)
{
    SDL_Event *event = &window->__sdl_event;

    while(SDL_PollEvent(event) > 0) 
    {
        switch (event->type) 
        {
            case SDL_WINDOWEVENT:
                __window_handle_sdlwindow_event(window, event);
            break;

            //NOTE: Here a mouse held down state is triggered if its just pressed and the mouse moved after.
            case SDL_MOUSEMOTION:
                __mouse_update_position(window);
            break;

            case SDL_MOUSEBUTTONDOWN:
                if (window->mouse_handler.just_pressed == false && window->mouse_handler.is_held == false) {

                    SDL_Log("Window (%s) MOUSE_DOWN\n", window->title);
                    window->mouse_handler.just_pressed  = true;

                } else if (window->mouse_handler.just_pressed == true && window->mouse_handler.is_held == false) {

                    SDL_Log("Window (%s) MOUSE_HELD_DOWN\n", window->title);
                    window->mouse_handler.just_pressed  = false;
                    window->mouse_handler.is_held = true;
                } 
            break;

            case SDL_MOUSEBUTTONUP:
                SDL_Log("Window (%s) MOUSE_UP\n", window->title);
                window->mouse_handler.just_pressed  = false;
                window->mouse_handler.is_held       = false;
            break;

            case SDL_KEYDOWN:
                __keyboard_update_buffers(window, SDL_KEYDOWN, event->key.keysym.scancode);
            break;

            case SDL_KEYUP:
                __keyboard_update_buffers(window, SDL_KEYUP, event->key.keysym.scancode);
            break;

            //default:
                //SDL_ShowSimpleMessageBox(0, "ERROR", "Key not accounted for", window->__sdl_window);
                //window->is_open = false;
        }
    }
}

void window_render_stuff(window_t *window, void (*render)(void *), void *arg)
{
    if (window == NULL) eprint("window argument is null");

    window_update_user_input(window);

#ifdef __gl_h_
    SDL_GL_MakeCurrent(window->__sdl_window, window->__glcontext);
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
            window->__sdl_surface, 
            NULL, 
            SDL_MapRGB(
                window->__sdl_surface->format, 
                color[0], color[1], color[2])
            );
#endif 

    render(arg);



#ifdef __gl_h_
    SDL_GL_SwapWindow(window->__sdl_window);
#else
    SDL_UpdateWindowSurface(window->__sdl_window);
#endif

}


void window_subwindow_destroy(window_t *subwindow)
{
    assert(subwindow);

#   ifdef __gl_h_
        SDL_GL_DeleteContext(subwindow->__glcontext);
#   else 
        SDL_FreeSurface(subwindow->__sdl_surface);
        subwindow->__sdl_surface = NULL;
#   endif 

    SDL_DestroyWindow(subwindow->__sdl_window);
    subwindow->__sdl_window = NULL;

    free(subwindow);
    subwindow = NULL;
}

void window_destroy(void)
{
    if (!global_window) eprint("global window is null");

    window_t *window = global_window;

#ifdef __gl_h_
    SDL_GL_DeleteContext(window->__glcontext);
#else 
    SDL_FreeSurface(window->__sdl_surface);
    window->__sdl_surface = NULL;
#endif 

    if (window->__subwindow != NULL) {
        window_subwindow_destroy(window->__subwindow);
        window->is_subwindow_active = false;
    }

    SDL_DestroyWindow(window->__sdl_window);
    SDL_Quit();

    window->__sdl_window = NULL;
    window = NULL;

    free(global_window);
    window = NULL;
}


#endif 
