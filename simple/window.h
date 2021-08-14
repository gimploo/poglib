#ifndef _SIMPLE_WINDOW_H_
#define _SIMPLE_WINDOW_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#ifdef __gl_h_
#include <SDL2/SDL_opengl.h>
#endif

#include "../math/la.h"

#define DEFAULT_BACKGROUND_COLOR (vec4f_t) {0.0f, 1.0f, 0.0f, 0.0f}
#define SDL_FLAGS u32

/*==============================================================================
 // Window library that is wrapper around SDL2 
==============================================================================*/

typedef void (*render_func) (void*);

typedef struct Mouse Mouse;
struct Mouse {

    bool    is_active;
    bool    is_dragged;
    vec2f_t position;
};

typedef struct window_t window_t;
struct window_t {

    bool is_open;
    u64 width, height;

    SDL_Window *window_handle; // initializes the window 

    SDL_Event event;           // handles the user inputs

    Mouse mouse_handler;

    vec4f_t background_color;

#ifndef __gl_h_                 // initializes the renderer
    SDL_Surface *surface_handle;
#else 
    SDL_GLContext gl_context;
#endif 

};


/*----------------------------------------------------------------------
 // Declarations
----------------------------------------------------------------------*/

window_t        window_init(size_t width, size_t height, SDL_FLAGS flags);
void            window_set_background(window_t *window, vec4f_t color);

//NOTE:(macro)  window_gl_render_begin(window_t *window)
//NOTE:(macro)  window_gl_render_end(window_t *window)
void            window_render(window_t *window, render_func render, void * arg);

void            window_process_user_input(window_t *window);

void            window_destroy(window_t *window);


/*-------------------------------------------------------------------------
 // Implementations
-------------------------------------------------------------------------*/

#define window_gl_render_begin(pwindow) {\
    glClearColor(\
            (pwindow)->background_color.cmp[0],\
            (pwindow)->background_color.cmp[1],\
            (pwindow)->background_color.cmp[2],\
            (pwindow)->background_color.cmp[3]\
    );\
    glClear(GL_COLOR_BUFFER_BIT);\
}

#define window_gl_render_end(pwindow) SDL_GL_SwapWindow((pwindow)->window_handle)

void window_set_background(window_t *window, vec4f_t color) 
{
    if(window == NULL) eprint("window argument is null");

    window->background_color = color;
}


static inline vec2f_t __mouse_get_position(window_t *window)
{
    if (window == NULL) eprint("window argument is null");

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
    if (window == NULL) eprint("window argument is null");

    window->mouse_handler.position =  __mouse_get_position(window);
}

static inline Mouse __mouse_init(window_t *window)
{
    if (window == NULL) eprint("window argument is null");

    return (Mouse) {
        .is_active = false,
        .is_dragged = false,
        .position = __mouse_get_position(window)
    };
}



window_t window_init(size_t width, size_t height, SDL_FLAGS flags)
{
    window_t output         = {0};
    SDL_FLAGS WinFlags      = 0;
    output.is_open          = true;
    output.width            = width;
    output.height           = height;
    output.background_color = DEFAULT_BACKGROUND_COLOR;
    output.mouse_handler    = __mouse_init(&output);

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
    output.window_handle = SDL_CreateWindow("window_t", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, WinFlags);
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

void window_process_user_input(window_t *window)
{
    if (window == NULL)  eprint("window argument is null");

    SDL_Event event = window->event;
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
                switch (event.key.keysym.sym)
                {
                    case SDLK_LEFT:  
                    case SDLK_RIGHT: 
                    case SDLK_UP:
                    case SDLK_DOWN:
                        break;
                    case SDLK_F1:
                        break;
                    default:
                        SDL_ShowSimpleMessageBox(0, "ERROR", "Key not accounted for", window->window_handle);
                        window->is_open = false;
                }
                break;
        }

    }
}


void window_render(window_t *window, render_func render, void *arg)
{
    if (window == NULL) eprint("window argument is null");

#ifdef __gl_h_
    glClearColor(
            window->background_color.cmp[0], 
            window->background_color.cmp[1],
            window->background_color.cmp[2],
            window->background_color.cmp[3]
    );
    glClear(GL_COLOR_BUFFER_BIT);
#endif 

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

    SDL_DestroyWindow(window->window_handle);
    SDL_Quit();
    
}


#endif // __SIMPLE_WINDOW_H_
