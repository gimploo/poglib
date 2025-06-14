#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
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

typedef enum {

    SDL_MOUSESTATE_NONE,
    SDL_MOUSESTATE_JUST_PRESSED,
    SDL_MOUSESTATE_PRESSED,
    SDL_MOUSESTATE_RELEASED,
    SDL_MOUSESTATE_HELD,

} sdl_mousestate;

typedef enum {

    SDL_MOUSEWHEEL_NONE = 0,
    SDL_MOUSEWHEEL_UP,
    SDL_MOUSEWHEEL_DOWN,
    SDL_MOUSEWHEEL_LEFT,
    SDL_MOUSEWHEEL_RIGHT,

} sdl_mousewheelstate;

typedef enum {

    SDL_MOUSEBUTTON_NONE,
    SDL_MOUSEBUTTON_LEFT,
    SDL_MOUSEBUTTON_MIDDLE,
    SDL_MOUSEBUTTON_RIGHT,
    SDL_MOUSEBUTTON_X1,
    SDL_MOUSEBUTTON_X2,

} sdl_mousebuttontype;

typedef struct window_t {

    bool                is_open;
    const char          *title;
    u64                 width; 
    u64                 height;
    vec4f_t             background_color;

    struct {

        sdl_mousebuttontype button;
        sdl_mousestate      state;
        vec2f_t             norm_position;
        vec2i_t             position;
        vec2i_t             rel;

        struct {
            sdl_mousewheelstate state;
        } wheel;

    } mouse;

    struct {

        bool     keystate[SDL_NUM_SCANCODES]; 
        bool     just_pressed[SDL_NUM_SCANCODES]; 
        bool     is_held[SDL_NUM_SCANCODES];

    } keyboard;

    struct {

        bool                is_active;
        struct window_t     *window;        // Holds subwindow address
                                      
    } subwindow;

    struct {
        SDL_Keycode         key; 
        SDL_EventType       kstate;
    } thisframe;

    u32                 __sdl_window_id;    // useful for managing multiple windows
    SDL_Window          *__sdl_window;      // initializes the window 
    SDL_Event           __sdl_event;
#ifndef __gl_h_                       
    SDL_Surface         *__sdl_surface;
#else 
    SDL_GLContext       __glcontext;
#endif 

} window_t ;

global window_t     *global_window = NULL;

window_t *          window_init(const char *title, u64 width, u64 height, const u32 SDL_flags);
#define             window_get_current_active_window() global_window

void                window_update_user_input(window_t *window);
void                window_set_background(window_t *window, vec4f_t color);
void                window_update_title(window_t *window, const char *title);

bool                window_keyboard_is_key_just_pressed(window_t *window, SDL_Keycode key);
bool                window_keyboard_is_key_held(window_t *window, SDL_Keycode key);
bool                window_keyboard_is_key_pressed(window_t *window, SDL_Keycode key);

bool                window_mouse_button_just_pressed(window_t *window, sdl_mousebuttontype button);
bool                window_mouse_button_is_pressed(window_t *window, sdl_mousebuttontype button);
bool                window_mouse_button_is_held(window_t *window, sdl_mousebuttontype button);

bool                window_mouse_wheel_is_scroll_up(window_t *w);
bool                window_mouse_wheel_is_scroll_down(window_t *w);
bool                window_mouse_wheel_is_scroll_left(window_t *w);
bool                window_mouse_wheel_is_scroll_right(window_t *w);

#define             window_mouse_get_norm_position(PWINDOW)                     (PWINDOW)->mouse.norm_position
#define             window_mouse_get_position(PWINDOW)                          (PWINDOW)->mouse.position

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

#ifndef IGNORE_SDL_WINDOW_IMPLEMENTATION

#define DEFAULT_BACKGROUND_COLOR (vec4f_t ){ 1.0f, 0.0f, 0.0f, 1.0f}

bool window_mouse_button_just_pressed(
        window_t *window, 
        sdl_mousebuttontype button)
{
  return window->mouse.state == SDL_MOUSESTATE_JUST_PRESSED 
         && window->mouse.button == button;
}

bool window_mouse_button_is_pressed(
        window_t *window,
        sdl_mousebuttontype button)
{
  return window->mouse.button == button 
         && (window->mouse.state == SDL_MOUSESTATE_JUST_PRESSED 
             || window->mouse.state == SDL_MOUSESTATE_PRESSED
             || window->mouse.state == SDL_MOUSESTATE_HELD);
}

bool window_mouse_button_is_held(
        window_t *window,
        sdl_mousebuttontype button)
{
  return window->mouse.state == SDL_MOUSESTATE_HELD &&
         window->mouse.button == button;
}

bool window_mouse_button_is_released(
        window_t *window,
        sdl_mousebuttontype button)
{
  return window->mouse.state == SDL_MOUSESTATE_RELEASED &&
         window->mouse.button == button;
}

bool window_keyboard_is_key_just_pressed(window_t *window, SDL_Keycode key)
{
    bool output = window->keyboard.just_pressed[SDL_GetScancodeFromKey(key)];
    // window->keyboard.just_pressed[SDL_GetScancodeFromKey(key)] = false;
    return output;
}

bool window_keyboard_is_key_held(window_t *window, SDL_Keycode key)
{
    bool output = window->keyboard.is_held[SDL_GetScancodeFromKey(key)];
    // window->keyboard.is_held[SDL_GetScancodeFromKey(key)] = false;
    return output;
}

bool window_keyboard_is_key_pressed(window_t *window, SDL_Keycode key)
{
    bool output = window->keyboard.is_held[SDL_GetScancodeFromKey(key)] 
                    || window->keyboard.just_pressed[SDL_GetScancodeFromKey(key)];

    // window->keyboard.just_pressed[SDL_GetScancodeFromKey(key)] = false;
    // window->keyboard.is_held[SDL_GetScancodeFromKey(key)] = false;

    return output;
}

bool window_mouse_wheel_is_scroll_up(window_t *w)
{
    if(w->mouse.wheel.state == SDL_MOUSEWHEEL_UP) {
        w->mouse.wheel.state = SDL_MOUSEWHEEL_NONE;
        return true;
    }
    return false;
}
bool window_mouse_wheel_is_scroll_down(window_t *w)
{
    if(w->mouse.wheel.state == SDL_MOUSEWHEEL_DOWN) {
        w->mouse.wheel.state = SDL_MOUSEWHEEL_NONE;
        return true;
    }
    return false;
}
bool window_mouse_wheel_is_scroll_right(window_t *w)
{
    if(w->mouse.wheel.state == SDL_MOUSEWHEEL_RIGHT) {
        w->mouse.wheel.state = SDL_MOUSEWHEEL_NONE;
        return true;
    }
    return false;
}
bool window_mouse_wheel_is_scroll_left(window_t *w)
{
    if(w->mouse.wheel.state == SDL_MOUSEWHEEL_LEFT) {
        w->mouse.wheel.state = SDL_MOUSEWHEEL_NONE;
        return true;
    }
    return false;
}

#define __impl_window_subwindow_gl_render_begin(PWINDOW) do {\
\
    SDL_GL_MakeCurrent((PWINDOW)->__sdl_window, (PWINDOW)->__glcontext);\
    GL_CHECK(glClearColor(\
            (PWINDOW)->background_color.raw[0],\
            (PWINDOW)->background_color.raw[1],\
            (PWINDOW)->background_color.raw[2],\
            (PWINDOW)->background_color.raw[3]\
    ));\
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));\
    GL_CHECK(glEnable(GL_DEPTH_TEST));\
    GL_CHECK(glEnable(GL_BLEND));\
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));\
    GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));\
\
} while(0)

#define __impl_window_gl_render_begin(PWINDOW) do {\
\
    GL_CHECK(glViewport(0, 0, (PWINDOW)->width, (PWINDOW)->height));\
    GL_CHECK(glClearColor(\
            (PWINDOW)->background_color.raw[0],\
            (PWINDOW)->background_color.raw[1],\
            (PWINDOW)->background_color.raw[2],\
            (PWINDOW)->background_color.raw[3]\
    ));\
    GL_CHECK(glEnable(GL_BLEND));\
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));\
    GL_CHECK(glEnable(GL_DEPTH_TEST));\
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));\
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

    f32 normalizedX = 2.f *  ((f32) x / window->width) - 1.f;
    f32 normalizedY = 1.f - 2.f * ((f32) y / window->height);
    pos.x = normalizedX;
    pos.y = normalizedY;

    SDL_Log("Mouse pos := (%f, %f)\n", normalizedX, normalizedY);

    window->mouse.position = (vec2i_t ){ x, y };
    window->mouse.norm_position = pos;

}


static inline window_t __subwindow_init(const char *title, u64 width, u64 height, const u32 flags) 
{   
    window_t output         = {0};
    output.title            = title;
    output.is_open          = true;
    output.width            = width;
    output.height           = height;
    output.background_color = DEFAULT_BACKGROUND_COLOR;

    output.subwindow.window = NULL;
    output.subwindow.is_active = false;

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
    SDL_GL_MakeCurrent(output.__sdl_window, output.__glcontext);

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) eprint("GLEW Error: %s\n", glewGetErrorString(glewError));

    printf("[OUTPUT] Using OpenGL render\n");

#else 
    output.__sdl_surface = SDL_GetWindowSurface(output.__sdl_window);
    if (!output.__sdl_surface) eprint("SDL Error: %s\n", SDL_GetError());

    printf("[OUTPUT] Using standard sdl2 render\n");
#endif

    output.__sdl_window_id  = SDL_GetWindowID(output.__sdl_window);

    __mouse_update_position(&output);

    return output;

}

window_t * window_subwindow_init(window_t *parent, const char *title, u64 width, u64 height, const u32 flags)
{
    if (parent == NULL) eprint("parent window argument is null");

    window_t *subwindow = (window_t *)malloc(sizeof(window_t));
    if (subwindow == NULL) eprint("subwindow malloc failed");

    *subwindow                 = __subwindow_init(title, width, height, flags);
    parent->subwindow.window   = subwindow;

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
    win.mouse.state = SDL_MOUSESTATE_NONE;

    win.subwindow.window = NULL;
    win.subwindow.is_active = false;

    u32 WinFlags = 0;
#ifdef __gl_h_
    WinFlags = SDL_WINDOW_OPENGL;
#endif

    if (SDL_Init(flags) == -1) eprint("SDL Error: %s\n", SDL_GetError());

    win.__sdl_window = SDL_CreateWindow(
            win.title, 
            SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, width, height, WinFlags);

    if (!win.__sdl_window) eprint("SDL Error: %s\n", SDL_GetError());

    if (!SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1"))
        eprint("SDL Error: %s", SDL_GetError());

#ifdef __gl_h_

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4))
        eprint("SDL GL Error: %s\n", SDL_GetError());

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3))
        eprint("SDL GL Error: %s\n", SDL_GetError());

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
                        SDL_GL_CONTEXT_PROFILE_CORE))
        eprint("SDL GL Error: %s\n", SDL_GetError());

    if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))
        eprint("SDL GL Error: %s\n", SDL_GetError());

    if (SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24))
        eprint("SDL GL Error: %s\n", SDL_GetError());

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
            SDL_GL_CONTEXT_PROFILE_COMPATIBILITY))
        eprint("SDL GL Error: %s\n", SDL_GetError());

#endif 


#ifdef __gl_h_
    glewExperimental = true; // if using GLEW version 1.13 or earlier
    win.__glcontext = SDL_GL_CreateContext(win.__sdl_window);
    SDL_GL_SetSwapInterval(0);
    if (!win.__glcontext) eprint("SDL GL Error: %s\n", SDL_GetError());

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) eprint("GLEW Error: %s\n", glewGetErrorString(glewError));

    printf("[OUTPUT] Using OpenGL renderer version (%s)\n", glGetString(GL_VERSION));


#else 
    win.__sdl_surface = SDL_GetWindowSurface(win.__sdl_window);
    if (!win.__sdl_surface) eprint("SDL Error: %s\n", SDL_GetError());

    printf("[OUTPUT] Using standard sdl2 renderer\n");
#endif

    win.__sdl_window_id = SDL_GetWindowID(win.__sdl_window);

    global_window = (window_t *)calloc(1, sizeof(window_t ));
    assert(global_window);

    *global_window = win;

    return global_window;

}

INTERNAL void __window_handle_sdlwindow_event(window_t *window, SDL_Event *event) 
{
    if (window->__sdl_window_id == event->window.windowID) {

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
                  window->mouse.state = SDL_MOUSESTATE_NONE;
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

  } else if (window->subwindow.window->__sdl_window_id == event->window.windowID) {

    switch (event->window.event) 
    {
        case SDL_WINDOWEVENT_CLOSE:
          SDL_Log("Window (%s) close requested",
                  window->subwindow.window->title);
          window->subwindow.window->is_open = false;
          break;

        case SDL_WINDOWEVENT_SHOWN:
          SDL_Log("Window (%s) shown", window->subwindow.window->title);
          break;

        case SDL_WINDOWEVENT_HIDDEN:
          SDL_Log("Window (%s) hidden", window->subwindow.window->title);
          break;

        case SDL_WINDOWEVENT_EXPOSED:
          SDL_Log("Window (%s) exposed", window->subwindow.window->title);
          break;

        case SDL_WINDOWEVENT_MOVED:
          SDL_Log("Window (%s) moved to %d,%d", window->subwindow.window->title,
                  event->window.data1, event->window.data2);
          break;

        case SDL_WINDOWEVENT_RESIZED:
          SDL_Log("Window (%s) resized to %dx%d",
                  window->subwindow.window->title, event->window.data1,
                  event->window.data2);
          break;

        case SDL_WINDOWEVENT_SIZE_CHANGED:
          SDL_Log("Window (%s) size changed to %dx%d",
                  window->subwindow.window->title, event->window.data1,
                  event->window.data2);
          break;

        case SDL_WINDOWEVENT_MINIMIZED:
          SDL_Log("Window (%s) minimized", window->subwindow.window->title);
          break;

        case SDL_WINDOWEVENT_MAXIMIZED:
          SDL_Log("Window (%s) maximized", window->subwindow.window->title);
          break;

        case SDL_WINDOWEVENT_RESTORED:
          SDL_Log("Window (%s) restored", window->subwindow.window->title);
          break;

        case SDL_WINDOWEVENT_ENTER:
          SDL_Log("Mouse entered window (%s)", window->subwindow.window->title);
          window->subwindow.is_active = true;
          break;

        case SDL_WINDOWEVENT_LEAVE:
          SDL_Log("Mouse left window (%s)", window->subwindow.window->title);
          window->subwindow.is_active = false;
          break;

        case SDL_WINDOWEVENT_FOCUS_GAINED:
          SDL_Log("Window (%s) gained keyboard focus",
                  window->subwindow.window->title);
          window->subwindow.is_active = true;
          break;

        case SDL_WINDOWEVENT_FOCUS_LOST:
          SDL_Log("Window (%s) lost keyboard focus",
                  window->subwindow.window->title);
          window->subwindow.is_active = false;
          break;

        case SDL_WINDOWEVENT_TAKE_FOCUS:
          SDL_Log("Window (%s) is offered a focus",
                  window->subwindow.window->title);
          window->subwindow.is_active = true;
          break;

        case SDL_WINDOWEVENT_HIT_TEST:
          SDL_Log("Window (%s) has a special hit test",
                  window->subwindow.window->title);
          break;

        default:
          SDL_Log("Window (%s) got unknown event %d",
                  window->subwindow.window->title, event->window.event);
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
            if (window->subwindow.is_active) {

                SDL_Log("Window (%s) KEY_DOWN: %s\n", window->subwindow.window->title, SDL_GetScancodeName(key));

                if (window->subwindow.window->keyboard.keystate[key] == true) { 

                    SDL_Log("Window (%s) KEY_HELD: %s\n", window->subwindow.window->title, SDL_GetScancodeName(key));
                    window->subwindow.window->keyboard.is_held[key]        = true;
                    window->subwindow.window->keyboard.just_pressed[key]   = false;

                } else if (window->subwindow.window->keyboard.keystate[key] == false) {

                    SDL_Log("Window (%s) KEY_JUST_PRESSED: %s\n", window->subwindow.window->title, SDL_GetScancodeName(key));
                    window->subwindow.window->keyboard.just_pressed[key]   = true;
                    window->subwindow.window->keyboard.is_held[key]        = false;

                }

                window->subwindow.window->keyboard.keystate[key] = true;

            } else {

                SDL_Log("Window (%s) KEY_DOWN: %s\n", window->title, SDL_GetScancodeName(key));
                if (window->keyboard.keystate[key] == true) { 

                    SDL_Log("Window (%s) KEY_HELD: %s\n", window->title, SDL_GetScancodeName(key));
                    window->keyboard.is_held[key]        = true;
                    window->keyboard.just_pressed[key]   = false;

                } else if (window->keyboard.keystate[key] == false) {

                    SDL_Log("Window (%s) KEY_JUST_PRESSED: %s\n", window->title, SDL_GetScancodeName(key));
                    window->keyboard.just_pressed[key]   = true;
                    window->keyboard.is_held[key]        = false;

                    window->keyboard.keystate[key] = true;
                }
            }
        break;

        case SDL_KEYUP:
            if (window->subwindow.is_active) {
                SDL_Log("Window (%s) KEY_UP: %s\n", window->subwindow.window->title, SDL_GetScancodeName(key));
                window->subwindow.window->keyboard.is_held[key]        = false;
                window->subwindow.window->keyboard.just_pressed[key]   = false;
                window->subwindow.window->keyboard.keystate[key]       = false;

            } else {
                SDL_Log("Window (%s) KEY_UP: %s\n", window->title, SDL_GetScancodeName(key));
                window->keyboard.is_held[key]        = false;
                window->keyboard.just_pressed[key]   = false;
                window->keyboard.keystate[key]       = false;
            }
        break;
    }
}


void window_subwindow_render_stuff(window_t *subwindow, void (*stuff)(void *), void *arg)
{
#ifdef __gl_h_
        SDL_GL_MakeCurrent(subwindow->__sdl_window, subwindow->__glcontext);
        glClearColor(
                subwindow->background_color.raw[0], 
                subwindow->background_color.raw[1],
                subwindow->background_color.raw[2],
                subwindow->background_color.raw[3]
        );
        glClear(GL_COLOR_BUFFER_BIT);
#else 
        u8 color[3] = {
            (u8) denormalize(subwindow->background_color.x, 0, 255),
            (u8) denormalize(subwindow->background_color.y, 0, 255),
            (u8) denormalize(subwindow->background_color.z, 0, 255)
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

#define SDL_KEYSTATE_UNKNOWN SDL_FIRSTEVENT

void window_update_user_input(window_t *window)
{
    SDL_Event *event = &window->__sdl_event;

    if (window->mouse.state == SDL_MOUSESTATE_JUST_PRESSED ) {
        window->mouse.state = SDL_MOUSESTATE_PRESSED;
    } else if (window->mouse.state == SDL_MOUSESTATE_RELEASED) {
        window->mouse.state = SDL_MOUSESTATE_NONE;
    }

    memset(&window->mouse.rel, 0, sizeof(window->mouse.rel));

    window->thisframe.key = SDLK_UNKNOWN;
    window->thisframe.kstate = SDL_KEYSTATE_UNKNOWN;

    while(SDL_PollEvent(event) > 0) 
    {
        switch (event->type) 
        {
            case SDL_WINDOWEVENT:
                __window_handle_sdlwindow_event(window, event);
            break;

            case SDL_MOUSEWHEEL:
                if(event->wheel.y > 0)      window->mouse.wheel.state = SDL_MOUSEWHEEL_UP;
                else if(event->wheel.y < 0) window->mouse.wheel.state = SDL_MOUSEWHEEL_DOWN;
                else if(event->wheel.x > 0) window->mouse.wheel.state = SDL_MOUSEWHEEL_RIGHT;
                else if(event->wheel.x < 0) window->mouse.wheel.state = SDL_MOUSEWHEEL_LEFT;
            break;

            //NOTE: Here a mouse held down state is triggered if its just pressed and the mouse moved after.
            case SDL_MOUSEMOTION:
                __mouse_update_position(window);
                const vec2i_t rel = { -1 * event->motion.xrel, -1 * event->motion.yrel };
                window->mouse.rel = rel;
                switch(window->mouse.state)
                {
                    case SDL_MOUSESTATE_RELEASED:
                    case SDL_MOUSESTATE_NONE:
                        window->mouse.state = SDL_MOUSESTATE_NONE;
                    break;

                    case SDL_MOUSESTATE_JUST_PRESSED:
                    case SDL_MOUSESTATE_PRESSED:
                    case SDL_MOUSESTATE_HELD:
                        window->mouse.state = SDL_MOUSESTATE_HELD;
                    break;
                }
            break;

            case SDL_MOUSEBUTTONUP:
                SDL_Log("Window (%s) MOUSE_DOWN\n", window->title);
                switch(window->mouse.state)
                {
                    case SDL_MOUSESTATE_RELEASED:
                    case SDL_MOUSESTATE_NONE:
                        window->mouse.state = SDL_MOUSESTATE_NONE;
                    break;

                    case SDL_MOUSESTATE_JUST_PRESSED:
                    case SDL_MOUSESTATE_PRESSED:
                    case SDL_MOUSESTATE_HELD:
                        window->mouse.state = SDL_MOUSESTATE_RELEASED;
                    break;
                }

            break;

            case SDL_MOUSEBUTTONDOWN:
                SDL_Log("Window (%s) MOUSE_UP\n", window->title);
                switch(window->mouse.state)
                {
                    case SDL_MOUSESTATE_RELEASED:
                    case SDL_MOUSESTATE_NONE:
                        window->mouse.state = SDL_MOUSESTATE_JUST_PRESSED;
                        window->mouse.button =
                            (sdl_mousebuttontype)event->button.button;
                    break;
                }
            break;

            case SDL_KEYDOWN:
                window->thisframe.key      = event->key.keysym.sym;
                window->thisframe.kstate    = SDL_KEYDOWN;
                if (event->key.repeat) 
                    break;

                __keyboard_update_buffers(window, SDL_KEYDOWN, event->key.keysym.scancode);
            break;

            case SDL_KEYUP:
                window->thisframe.key      = event->key.keysym.sym;
                window->thisframe.kstate    = SDL_KEYUP;

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
            window->background_color.raw[0], 
            window->background_color.raw[1],
            window->background_color.raw[2],
            window->background_color.raw[3]
    );
    glClear(GL_COLOR_BUFFER_BIT);
#else 
    u8 color[3] = {
        (u8) denormalize(window->background_color.x, 0, 255),
        (u8) denormalize(window->background_color.y, 0, 255),
        (u8) denormalize(window->background_color.z, 0, 255)
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

    if (window->subwindow.window != NULL) {
        window_subwindow_destroy(window->subwindow.window);
        window->subwindow.is_active = false;
    }

    SDL_DestroyWindow(window->__sdl_window);
    SDL_Quit();

    window->__sdl_window = NULL;
    window = NULL;

    free(global_window);
    window = NULL;
}


#endif 
