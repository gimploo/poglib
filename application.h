#pragma once
#include "./basic.h"
#include "simple/glrenderer2d.h"
#include "simple/window.h"
#include "application/stopwatch.h"
#include "ecs/entitymanager.h"
#include "font/glfreetypefont.h"




typedef struct application_t application_t;

typedef u8 state_t;

#define         application_init(PWIN) __impl_application_init((PWIN), (void *)app_init, (void *)app_update, (void *)app_render, (void *)app_shutdown)

#define         application_set_font(PAPP, FONT)        (PAPP)->__fontrenderer = FONT
#define         application_pass_game(PAPP, PGAME)      (PAPP)->content = PGAME

void            application_run(application_t *app);

#define         application_update_state(PAPP, STATE) (PAPP)->state = STATE
#define         application_get_dt(PAPP)  (PAPP)->__timer.dt
#define         application_get_fps(PAPP) (PAPP)->__timer.fps







#ifndef IGNORE_APPLICATION_IMPLEMENTATION

struct application_t {

    state_t             state;
    window_t            *__window_handle;
    stopwatch_t         __timer;
    glfreetypefont_t    *__fontrenderer;

    union {
        void *content;
        void *game;
        void *engine;
    };


    void (* init)(struct application_t *);
    void (*update)(struct application_t *);
    void (*render)(struct application_t *);
    void (*shutdown)(struct application_t *);
};

    


application_t __impl_application_init(window_t *window, void (* init)(struct application_t*), void (*update)(struct application_t*),void (*render)(struct application_t*), void (*shutdown)(struct application_t*))
{
    return (application_t) {
        .state = 0,
        .__window_handle = window,
        .__timer = stopwatch_init(), 
        .__fontrenderer = NULL,
        .content = NULL,
        .init = init,
        .update = update,
        .render = render,
        .shutdown = shutdown
    };
}



void application_run(application_t *app)
{
#ifdef DEBUG
    dbg_init();
#endif

    if (app == NULL) eprint("application argument is null");
    assert(app->__window_handle);
    
    window_t *win = app->__window_handle;
    stopwatch_t *timer = &app->__timer;

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

        timer->__last   = timer->__now;
        timer->__now    = (f32)SDL_GetTicks();
        timer->dt = (timer->__now - timer->__last) / 1000.0f;

        if (timer->dt > 0.25f) timer->dt = 0.25f;

        timer->accumulator += timer->dt;

        while(timer->accumulator >= timer->dt && timer->dt != 0.0f)
        {
            st_previous = st_current;

            application_update_state(app, st_current);

            window_update_user_input(win);
            app->update(app);

            timer->accumulator -= timer->dt;
        }

        const f32 alpha = timer->accumulator / timer->dt;


        state_t state   = st_current * alpha + st_previous * (1.0 - alpha);
        application_update_state(app, state);


        window_gl_render_begin(win);

            app->render(app);

        window_gl_render_end(win);


        // NOTE: for now i am capping at 60 fps 
        /*
         * u64 perf_end = SDL_GetPerformanceCounter();
         * timer->fps = 1.0f / ((perf_end - perf_start) / (f32)SDL_GetPerformanceFrequency());
         */

        //NOTE: here is a simple dt to fps converter dont think its accurate for physics stuff
        //need to do more research on delta time, especially the math behind it
        //
        timer->fps = 1.0f / timer->dt;
        SDL_Delay(floor(1000/60 - timer->dt));
    }

    printf("[!] APPLICATION SHUTDOWN!\n");
    app->shutdown(app);

#ifdef DEBUG
    dbg_destroy();
#endif

}

#endif 
