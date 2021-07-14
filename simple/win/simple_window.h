#ifndef _SIMPLE_WINDOW_H_
#define _SIMPLE_WINDOW_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#ifdef __gl_h_
#include <SDL2/SDL_opengl.h>
#endif


#define SDL_FLAGS u_int32_t



typedef struct my_window SimpleWindow;
struct my_window {

    bool is_window_open;
    SDL_Window *window_handle;

#ifndef __gl_h_ // if OpenGL is included
    SDL_Surface *surface_handle;
#else 
    SDL_GLContext gl_context;
#endif 

    SDL_Event event;

};




/*
 *
 * Function Declaration
 *
 */


SimpleWindow    window_init(SDL_FLAGS flags);
void            window_event_handler(SimpleWindow *window);
void            window_event_process_user_input(SimpleWindow *window);
void            window_destroy(SimpleWindow *window);


/*
 *
 * Function Definitions
 *
 */


SimpleWindow window_init(SDL_FLAGS flags)
{
    SimpleWindow output = {0};
    SDL_FLAGS WinFlags = 0;

    output.is_window_open = true;

#ifdef __gl_h_
    WinFlags = SDL_WINDOW_OPENGL;

    int major_ver, minor_ver;

    if ( !SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major_ver) || 
         !SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor_ver)) 
    {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }
    if ( !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major_ver) ||
         !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor_ver))
    {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
                        SDL_GL_CONTEXT_PROFILE_CORE))
    {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }

#endif 

    /*
     *
     * SDL init
     *
     */

    if (SDL_Init(flags) == -1) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }
    output.window_handle = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 680, 480, WinFlags);
    if (!output.window_handle) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }

    /*
     *
     * Surface context / Gl context
     *
     */

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

    glewExperimental = true; // if using GLEW version 1.13 or earlier
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(glewError));
        exit(1);
    }

#endif

    return output;

}

void window_event_handler(SimpleWindow *window)
{
    if (window == NULL) {
        fprintf(stderr, "%s: window argument is null\n", __func__);
        exit(1);
    }

    while (window->is_window_open) 
    {
    #ifdef __gl_h_
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    #endif 

        window_event_process_user_input(window);

    #ifdef __gl_h_
        SDL_GL_SwapWindow(window->window_handle);
    #else
        SDL_UpdateWindowSurface(window->window_handle);
    #endif
    }
}

void window_event_process_user_input(SimpleWindow *window)
{
    if (window == NULL) {
        fprintf(stderr, "%s: window argument is null\n", __func__);
        exit(1);
    }
    SDL_Event event = window->event;
    while(SDL_PollEvent(&event) > 0) 
    {
        switch (event.type) 
        {
            case SDL_QUIT:
                window->is_window_open = false;
                break;
        }

    }
}

void window_destroy(SimpleWindow *window)
{
    if (window == NULL) {
        fprintf(stderr, "%s: window argument is null\n", __func__);
        exit(1);
    }
    SDL_DestroyWindow(window->window_handle);
    SDL_Quit();
    
}

#endif // __SIMPLE_WINDOW_H_