#pragma once
#include "./basic.h"
#include "simple/glrenderer2d.h"
#include "simple/window.h"
#include "application/stopwatch.h"
#include "ecs/entitymanager.h"
#include "font/glfreetypefont.h"
#include "./poggen.h"


/*=============================================================================
                            - APPLICATION -
=============================================================================*/

#define state_t u8

typedef struct application_t {

    window_t            *__window;
    stopwatch_t         *__timer;
    glfreetypefont_t    *__fontrenderer;

    char                *window_title;
    u32                 window_width;
    u32                 window_height;

    state_t             state;

    union {
        void        *content;
        void        *game;
    };


    void (* init)(struct application_t *);
    void (*update)(struct application_t *);
    void (*render)(struct application_t *);
    void (*shutdown)(struct application_t *);

} application_t ;


void            application_run(application_t *app);

#define         application_set_font(PAPP, FONT)            (PAPP)->__fontrenderer = FONT
#define         application_set_background(PAPP, COLOR)     (PAPP)->__window->background_color = COLOR

#define         application_get_window(PAPP) (PAPP)->__window
#define         application_get_dt(PAPP)  (PAPP)->__timer->dt
#define         application_get_fps(PAPP) (PAPP)->__timer->fps

#define         application_update_state(PAPP, STATE)       (PAPP)->state = STATE


/*---------------------------------------------------------------------------*/


#ifndef IGNORE_APPLICATION_IMPLEMENTATION


void application_run(application_t *app)
{
#ifdef DEBUG
    dbg_init();
#endif

    if (app == NULL)                eprint("application argument is null");
    if (!app->window_title)         eprint("application title is missing ");
    if (app->window_width <= 0)     eprint("provide a proper width to the application");
    if (app->window_height <= 0)    eprint("provide a proper height to the application");
    if(!app->init)                  eprint("application init funciton is missing");
    if(!app->update)                eprint("application update function is missing");
    if(!app->render)                eprint("application render function is missing");
    if(!app->shutdown)              eprint("application shutdown function is missing");

    window_t * win = window_init(
            app->window_title, 
            app->window_width, 
            app->window_height, 
            SDL_INIT_EVERYTHING);
    assert(win);

    stopwatch_t timer = stopwatch();

    app->__window= win;
    app->__timer = &timer;
    app->__fontrenderer = NULL;

    // Initialize the content in the application
    printf("[!] APPLICATION INIT!\n");
    app->init(app);

    // states
    state_t st_current  = app->state, 
            st_previous = 0; 

    printf("[!] APPLICATION RUNNING!\n");
    // Update the content in the application
    while(win->is_open)
    {
        //u64 perf_start  = SDL_GetPerformanceCounter();

        timer.__last   = timer.__now;
        timer.__now    = (f32)SDL_GetTicks();
        timer.dt = (timer.__now - timer.__last) / 1000.0f;

        if (timer.dt > 0.25f) timer.dt = 0.25f;

        timer.accumulator += timer.dt;

        while(timer.accumulator >= timer.dt && timer.dt != 0.0f)
        {
            st_previous = st_current;

            application_update_state(app, st_current);

            SDL_Event *event = &win->__sdl_event;

            while(SDL_PollEvent(event) > 0) 
                if (event->type == SDL_QUIT) 
                    win->is_open = false;

            app->update(app);

            timer.accumulator -= timer.dt;
        }

        const f32 alpha = timer.accumulator / timer.dt;


        state_t state   = st_current * alpha + st_previous * (1.0 - alpha);
        application_update_state(app, state);


        window_gl_render_begin(win);

            app->render(app);

        window_gl_render_end(win);


        // NOTE: for now i am capping at 60 fps 
        /*
         * u64 perf_end = SDL_GetPerformanceCounter();
         * timer.fps = 1.0f / ((perf_end - perf_start) / (f32)SDL_GetPerformanceFrequency());
         */

        //NOTE: here is a simple dt to fps converter dont think its accurate for physics stuff
        //need to do more research on delta time, especially the math behind it
        //
        timer.fps = 1.0f / timer.dt;
        SDL_Delay(floor(1000/60 - timer.dt));
    }

    printf("[!] APPLICATION SHUTDOWN!\n");
    app->shutdown(app);

    window_destroy();

#ifdef DEBUG
    dbg_destroy();
#endif

}

#endif 
