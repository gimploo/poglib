#pragma once
#if defined(WINDOW_SDL)
#include <SDL2/SDL.h>
#elif defined(WINDOW_GLFW)
#include <GLFW/glfw3.h>
#else
#include <SDL2/SDL.h>
#endif

#include "../basic.h"
#include <math.h>
#include <limits.h>

/*=============================================================================
                        -- STOP WATCH (TIMER) --
=============================================================================*/

typedef struct stopwatch_t {

    f32 __now;
    f32 __last;
    f32 accumulator;

    f32 dt; // delta time in seconds
    f32 fps;

} stopwatch_t ;

stopwatch_t         stopwatch(void);
void                stopwatch_update(stopwatch_t *timer);
void                stopwatch_delay(const f32 ms);
f32                 stopwatch_get_tick(void);
#define             stopwatch_get_tick_in_seconds() (stopwatch_get_tick() / 1000.0f)


/*=============================================================================
                        -- IMPELENTATION --
=============================================================================*/

#ifndef IGNORE_STOPWATCH_IMPLEMENTATION

f32 stopwatch_get_tick(void)
{
#if defined(WINDOW_GLFW)
    return (f32)glfwGetTime() * 1000.0f;
#else
    return (f32)SDL_GetTicks();
#endif
}

void stopwatch_delay(const f32 ms)
{
#if defined(WINDOW_GLFW)
#   ifdef _WIN64
        Sleep(ms);
#   else
        usleep(ms);
#   endif
#else
    SDL_Delay( ms );
#endif
}

stopwatch_t stopwatch(void)
{
    stopwatch_t output = {0};
    output.__now = stopwatch_get_tick();
    output.dt = 0.01f;
    return output;
}

/*
     double t = 0.0;
    double dt = 0.01;

    double currentTime = hires_time_in_seconds();
    double accumulator = 0.0;

    State previous;
    State current;

    while ( !quit )
    {
        double newTime = time();
        double dt = newTime - currentTime;
        if ( dt > 0.25 )
            dt = 0.25;
        currentTime = newTime;

        accumulator += dt;

        while ( accumulator >= dt )
        {
            previousState = currentState;
            integrate( currentState, t, dt );
            t += dt;
            accumulator -= dt;
        }

        const double alpha = accumulator / dt;

        State state = currentState * alpha + 
            previousState * ( 1.0 - alpha );

        render( state );
    } 
 */ 

void stopwatch_update(stopwatch_t *timer)
{
    if (timer == NULL) eprint("dt argument is null");

    timer->__last   = timer->__now;
    timer->__now    = stopwatch_get_tick();
    timer->dt = (timer->__now - timer->__last) / 1000.0f;

    if (timer->dt > 0.25f) timer->dt = 0.25f;

    timer->accumulator += timer->dt;

}

#endif
