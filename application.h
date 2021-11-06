#pragma once


#include "simple/window.h"
#include "application/stopwatch.h"


typedef struct application_t application_t;

typedef u8 state_t;

application_t   application_init(window_t *window, void (*init)(void*), void (*update)(void*), void (*render)(void*));
void            application_run(application_t *app);
void            application_destroy(application_t *app);

#define application_update_state(PAPP, STATE) (PAPP)->state = STATE

#define application_get_dt(PAPP)  (PAPP)->timer.dt
#define application_get_fps(PAPP) (PAPP)->timer.fps

struct application_t {

    window_t *__window_handle;
    
    stopwatch_t timer;

    state_t state;

    void (*init)(void*);
    void (*update)(void*);
    void (*render)(void*);
};


application_t application_init(window_t *window, void (*init)(void*), void (*update)(void*), void (*render)(void*))
{
    if (window == NULL) eprint("window argument is null");
    if (init == NULL) eprint("init function pointer is null");
    if (update == NULL) eprint("update function pointer is null");

    return (application_t) {
        .__window_handle = window,
        .timer = stopwatch_init(), 
        .state = 0,
        .init = init,
        .update = update,
        .render = render
    };
}



void application_run(application_t *app)
{
    if (app == NULL) eprint("application argument is null");
    
    window_t *win = app->__window_handle;
    stopwatch_t *timer = &app->timer;

    // Initialize the content in the application
    app->init(NULL);

    // states
    state_t st_current  = app->state, 
            st_previous = 0; 

    // Update the content in the application
    while(win->is_open)
    {
        u64 perf_start  = SDL_GetPerformanceCounter();

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

            app->update(NULL);

            timer->accumulator -= timer->dt;
        }

        const f32 alpha = timer->accumulator / timer->dt;


        state_t state   = st_current * alpha + st_previous * (1.0 - alpha);
        application_update_state(app, state);


        app->render(NULL);


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
    application_destroy(app);
}

void application_destroy(application_t *app)
{
    if (app == NULL) eprint("application argument is null");

    window_destroy(app->__window_handle);

}
