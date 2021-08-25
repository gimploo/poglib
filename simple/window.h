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
#endif

#include "../math/la.h"
#include "../game/delta_time.h"

#define DEFAULT_BACKGROUND_COLOR (vec4f_t){{ 0.0f, 1.0f, 0.0f, 0.0f}}
#define SDL_FLAGS u32

/*==============================================================================
 // Window library that is wrapper around SDL2 
==============================================================================*/

typedef void (*render_func) (void*);

typedef struct __mouse_t {

    bool    is_active;
    bool    is_dragged;
    vec2f_t position;

} __mouse_t;

typedef struct __keyboard_t {

    //bool        is_active;
    //SDL_Keycode key;
    
    bool        key_pressed[SDL_NUM_SCANCODES];
    bool        key_held[SDL_NUM_SCANCODES];
    bool        key_released[SDL_NUM_SCANCODES];

} __keyboard_t;

typedef struct window_t {

    bool            is_open;
    const char      *title_name;
    u64             width; 
    u64             height;
    SDL_Window      *window_handle;             // initializes the window 
    __mouse_t       mouse_handler;
    __keyboard_t    keyboard_handler;
    vec4f_t         background_color;

#ifndef __gl_h_                                 // initializes the renderer
    SDL_Surface     *surface_handle;
#else 
    SDL_GLContext   gl_context;
#endif 

    game_loop_time_t time;

} window_t;

/*----------------------------------------------------------------------
 // Declarations
----------------------------------------------------------------------*/

window_t        window_init(const char *title, size_t width, size_t height, SDL_FLAGS flags);
void            window_set_background(window_t *window, vec4f_t color);
void            window_set_title(window_t *window, const char *title_name);

//NOTE:(macro)  window_keyboard_is_key_pressed(window_t *window, SDL_Keycode key) -> bool

//NOTE:(macro)  window_gl_render_begin(window_t *window)
//NOTE:(macro)  window_gl_render_end(window_t *window)
//NOTE:(macro)  window_game_loop(window_t *window)
//NOTE:(macro)  window_get_dt(window_t *window) -> f64
//NOTE:(macro)  window_get_fps(window_t *window) -> f64

void            window_render(window_t *window, render_func render, void * arg);

void            window_destroy(window_t *window);



/*-------------------------------------------------------------------------
 // Implementations
-------------------------------------------------------------------------*/

#define window_game_loop(pwindow)   while((pwindow)->is_open && game_loop_time_calculate(&(pwindow)->time))
#define window_cap_fps(pwindow)     SDL_Delay(floor(16.666f - (pwindow)->time.elapsed_in_ms))

#define window_get_dt(pwindow)      (pwindow)->time.dt_value
#define window_get_fps(pwindow)     (pwindow)->time.fps_value

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

void window_set_title(window_t *window, const char *title_name)
{
    if (window == NULL) eprint("window argument is null");

    window->title_name = title_name;
}

void window_set_background(window_t *window, vec4f_t color) 
{
    if(window == NULL) eprint("window argument is null");

    window->background_color = color;
}


static inline vec2f_t __mouse_get_position(window_t *window)
{
    int x, y;
    vec2f_t pos;

    SDL_GetMouseState(&x, &y);

    //printf("Mouse pos := (%d, %d)\n", x, y);

    f32 normalizedX = -1.0 + 2.0 *  (f32) x / window->width;
    f32 normalizedY = (1.0 - 2.0 * (f32) y / window->height);
    
    pos.cmp[X] = normalizedX;
    pos.cmp[Y] = normalizedY;
    
    //printf("Mouse pos := (%f, %f)\n", normalizedX, normalizedY);
    
    return pos;
}

static inline void  __mouse_update(window_t *window)
{
    window->mouse_handler.position =  __mouse_get_position(window);
}

static inline __mouse_t __mouse_init(window_t *window)
{
    return (__mouse_t) {
        .is_active = false,
        .is_dragged = false,
        .position = __mouse_get_position(window)
    };
}



window_t window_init(const char *title_name, size_t width, size_t height, SDL_FLAGS flags)
{
    window_t output         = {0};
    SDL_FLAGS WinFlags      = 0;
    output.title_name       = title_name;
    output.is_open          = true;
    output.width            = width;
    output.height           = height;
    output.background_color = DEFAULT_BACKGROUND_COLOR;
    output.mouse_handler    = __mouse_init(&output);
    output.time             = game_loop_time_init();
    output.keyboard_handler = (__keyboard_t  ){
        .key_pressed    = {false},
        .key_held       = {false},
        .key_released   = {false}
    };

#ifdef __gl_h_
    WinFlags = SDL_WINDOW_OPENGL;
    int major_ver, minor_ver;

    if ( !SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major_ver) || 
         !SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor_ver)) 
        eprint("Error: %s\n", SDL_GetError());

    if ( !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major_ver) ||
         !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor_ver))

        eprint("Error: %s\n", SDL_GetError());

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
                        SDL_GL_CONTEXT_PROFILE_CORE))
        eprint("Error: %s\n", SDL_GetError());
#endif 

    if (SDL_Init(flags) == -1) eprint("Error: %s\n", SDL_GetError());
    output.window_handle = SDL_CreateWindow(output.title_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, WinFlags);
    if (!output.window_handle) eprint("Error: %s\n", SDL_GetError());


#ifndef __gl_h_
    output.surface_handle = SDL_GetWindowSurface(output.window_handle);
    if (!output.surface_handle) eprint("Error: %s\n", SDL_GetError());
    printf("[OUTPUT] Using standard sdl2 render\n");

#else 
    glewExperimental = true; // if using GLEW version 1.13 or earlier

    output.gl_context = SDL_GL_CreateContext(output.window_handle);
    if (!output.gl_context) eprint("Error: %s\n", SDL_GetError());

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) eprint("Error: %s\n", glewGetErrorString(glewError));

    printf("[OUTPUT] Using OpenGL render\n");
#endif

    return output;

}

void __window_update_user_input(window_t *window)
{
    SDL_Event event;
    while(SDL_PollEvent(&event) > 0) 
    {
        switch (event.type) 
        {
            case SDL_QUIT:
                window->is_open = false;
            break;

            case SDL_MOUSEMOTION:
                __mouse_update(window);
                if (window->mouse_handler.is_active == true) window->mouse_handler.is_dragged = true; 
            break;

            case SDL_MOUSEBUTTONDOWN:
                __mouse_update(window);
                window->mouse_handler.is_active = true;
                window->mouse_handler.is_dragged = false;
            break;

            case SDL_MOUSEBUTTONUP:
                __mouse_update(window);
                window->mouse_handler.is_dragged = false;
                window->mouse_handler.is_active = false;
            break;

            case SDL_KEYDOWN:
                fprintf(stderr, "KEY_DOWN: %s\n", SDL_GetScancodeName(event.key.keysym.scancode));

                window->keyboard_handler.key_pressed[event.key.keysym.scancode]     = true;
                window->keyboard_handler.key_held[event.key.keysym.scancode]        = true;
                window->keyboard_handler.key_released[event.key.keysym.scancode]    = false;
                //window->keyboard_handler.is_active = true;
                //window->keyboard_handler.key = event.key.keysym.sym;
            break;

            case SDL_KEYUP:
                fprintf(stderr, "KEY_UP: %s\n", SDL_GetScancodeName(event.key.keysym.scancode));

                window->keyboard_handler.key_pressed[event.key.keysym.scancode]     = false;
                window->keyboard_handler.key_held[event.key.keysym.scancode]        = false;
                window->keyboard_handler.key_released[event.key.keysym.scancode]    = true;
                //window->keyboard_handler.is_active = false;
                //window->keyboard_handler.key = event.key.keysym.sym;
            break;

            //default:
                //SDL_ShowSimpleMessageBox(0, "ERROR", "Key not accounted for", window->window_handle);
                //window->is_open = false;
        }

    }
}

//#define window_keyboard_is_key_pressed(pwindow, KEY) ((pwindow)->keyboard_handler.is_active && ((pwindow)->keyboard_handler.key == KEY))
#define window_keyboard_is_key_pressed(pwindow, KEY) ((pwindow)->keyboard_handler.key_pressed[SDL_GetScancodeFromKey(KEY)] == true)
#define window_keyboard_is_key_released(pwindow, KEY) ((pwindow)->keyboard_handler.key_released[SDL_GetScancodeFromKey(KEY)] == true)
#define window_keyboard_is_key_held(pwindow, KEY) ((pwindow)->keyboard_handler.key_held[SDL_GetScancodeFromKey(KEY)] == true)


void window_render(window_t *window, render_func render, void *arg)
{
    if (window == NULL) eprint("window argument is null");

    __window_update_user_input(window);
#ifdef __gl_h_
    glClearColor(
            window->background_color.cmp[0], 
            window->background_color.cmp[1],
            window->background_color.cmp[2],
            window->background_color.cmp[3]
    );
    glClear(GL_COLOR_BUFFER_BIT);
#endif 

    game_loop_time_calculate(&window->time);

    render(arg);

#ifdef __gl_h_
    SDL_GL_SwapWindow(window->window_handle);
#else
    SDL_UpdateWindowSurface(window->window_handle);
#endif

}


void window_destroy(window_t *window)
{
    if (window == NULL) eprint("window argument is null");

#ifdef __gl_h_
    SDL_GL_DeleteContext(window->gl_context);
#else 
    SDL_DestroyRenderer(window->surface_handle);
    window->surface_handle = NULL;
#endif 
    SDL_DestroyWindow(window->window_handle);
    SDL_Quit();

    window->window_handle = NULL;
    window = NULL;
    
}


#endif // __SIMPLE_WINDOW_H_
