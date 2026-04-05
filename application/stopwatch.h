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

    f32 raw_dt; // delta time in seconds
    f32 fps;
    f32 fixed_dt;

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
    output.raw_dt = 0.01f;
    return output;
}

void stopwatch_update(stopwatch_t *timer)
{
    if (timer == NULL) eprint("timer argument is null");

    timer->__last = timer->__now;
    timer->__now  = stopwatch_get_tick();

    // Calculate raw delta in seconds
    f32 delta = (timer->__now - timer->__last) / 1000.0f;

    //INFO: The 'MAX_ACCUMULATOR' (0.25s) caps the amount of "catch-up" work the CPU 
    //does if a frame takes too long. This prevents the engine from freezing 
    //under heavy load by slowing down the game clock instead of hanging.
    if (delta > 0.25f) delta = 0.25f;

    timer->raw_dt = delta;
    timer->accumulator += delta;
}
#endif
