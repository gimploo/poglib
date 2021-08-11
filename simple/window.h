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

#define SDL_FLAGS u_int32_t
typedef void (*render_func) (void*);

typedef struct Mouse Mouse;
struct Mouse {

    bool    is_active;
    bool    is_dragged;
    vec2f   position;
};

typedef struct Window Window;
struct Window {

    bool is_open;
    u64 width, height;

    SDL_Window *window_handle; // initializes the window 

    SDL_Event event;           // handles the user inputs

    Mouse mouse_handler;

#ifndef __gl_h_                 // initializes the renderer
    SDL_Surface *surface_handle;
#else 
    SDL_GLContext gl_context;
#endif 

};





Window      window_init(size_t width, size_t height, SDL_FLAGS flags);
void        window_render(Window *window, render_func render, void * arg);
void        window_process_user_input(Window *window);
void        window_destroy(Window *window);





vec2f __mouse_get_position(Window *window)
{
    if (window == NULL) eprint("window argument is null");

    int x, y;
    vec2f pos;

    SDL_GetMouseState(&x, &y);

    //printf("Mouse pos := (%d, %d)\n", x, y);

    f32 normalizedX = -1.0 + 2.0 *  (f32) x / window->width;
    f32 normalizedY = (1.0 - 2.0 * (f32) y / window->height);
    
    pos.cmp[X] = normalizedX;
    pos.cmp[Y] = normalizedY;
    
    //printf("Mouse pos := (%f, %f)\n", normalizedX, normalizedY);
    
    return pos;
}

void  __mouse_update(Window *window)
{
    if (window == NULL) eprint("window argument is null");

    window->mouse_handler.position =  __mouse_get_position(window);
}

Mouse __mouse_init(Window *window)
{
    if (window == NULL) eprint("window argument is null");

    return (Mouse) {
        .is_active = false,
        .is_dragged = false,
        .position = __mouse_get_position(window)
    };
}



Window window_init(size_t width, size_t height, SDL_FLAGS flags)
{
    Window output = {0};
    SDL_FLAGS WinFlags = 0;
    output.is_open = true;
    output.width = width;
    output.height = height;

    output.mouse_handler = __mouse_init(&output);

    
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



    if (SDL_Init(flags) == -1) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(1);
    }
    output.window_handle = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, WinFlags);
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

    printf("[OUTPUT] Using standard sdl2 render\n");

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
    printf("[OUTPUT] Using OpenGL render\n");

#endif

    return output;

}

void window_process_user_input(Window *window)
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

void window_render(Window *window, render_func render, void *arg)
{
    if (window == NULL) {
        fprintf(stderr, "%s: window argument is null\n", __func__);
        exit(1);
    }

#ifdef __gl_h_
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif 

    render(arg);

#ifdef __gl_h_
    SDL_GL_SwapWindow(window->window_handle);
#else
    SDL_UpdateWindowSurface(window->window_handle);
#endif

}


void window_destroy(Window *window)
{
    if (window == NULL) {
        fprintf(stderr, "%s: window argument is null\n", __func__);
        exit(1);
    }
    SDL_DestroyWindow(window->window_handle);
    SDL_Quit();
    
}


#endif // __SIMPLE_WINDOW_H_
