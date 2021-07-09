#ifndef _OPEN_WINDOW_H_
#define _OPEN_WINDOW_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>


#define SDL_INIT_FLAGS u_int32_t



typedef struct my_window Window;
struct my_window {

    bool is_window_open;
    SDL_Window *window_handle;

#ifndef __gl_h_ // if OpenGL is included
    SDL_Surface *surface_handle;
#else 
    SDL_GLContext gl_context;
#endif 

    SDL_Event event;

    SDL_INIT_FLAGS flags;

};




/*
 *
 * Function Declaration
 *
 */


Window      window_init(SDL_INIT_FLAGS flags);
void        window_event_handler(Window *window);


/*
 *
 * Function Definitions
 *
 */


Window window_init(SDL_INIT_FLAGS flags)
{
    Window output = {0};

    output.is_window_open = true;

    if (SDL_Init(flags) == -1) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }
    output.window_handle = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 680, 480, 0);
    if (!output.window_handle) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }
#ifndef __gl_h_
    output.surface_handle = SDL_GetWindowSurface(output.window_handle);
    if (!output.surface_handle) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }
#else 
    output.gl_context = SDL_GL_CreateContext(output.window_handle);
    if (!output.gl_context) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
#endif

    return output;

}

void window_event_handler(Window *window)
{
    if (window == NULL) {
        fprintf(stderr, "%s: window argument is null\n", __func__);
        exit(1);
    }
    SDL_Event event = window->event;
    while (window->is_window_open) 
    {
#ifdef __glu_h_
        glClear(GL_COLOR_BUFFER_BIT);
#endif 
        while(SDL_PollEvent(&event) > 0) 
        {
            switch (event.type) 
            {
                case SDL_QUIT:
                    window->is_window_open = false;
                    break;
            }

        #ifndef __gl_h_
            SDL_UpdateWindowSurface(window->window_handle);
        #else
            SDL_GL_SwapWindow(window->window_handle);
        #endif
        }
    }
}

#endif // __OPEN_WINDOW_H_
