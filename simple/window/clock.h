#pragma once

#include <SDL2/SDL.h>
#include "../../basic.h"
#include <math.h>
#include <limits.h>

//FIXME: the fps is twice the actual value and i am not sure whether its the elapsed_in_ms
// calculations fault or not, probably it might be. 

typedef struct window_clock_t {

    u64 __now;
    u64 __last;
    u32 __frame_counter;
    f64 __delta_accumulator;

    f64 elapsed_in_ms; // delta time in ms
    f64 fps_value;

} window_clock_t;


window_clock_t window_clock_init(void)
{
    window_clock_t output = {0};
    output.__now = SDL_GetPerformanceCounter();
    return output;
}

bool window_clock_update(window_clock_t *timer)
{
    if (timer == NULL) eprint("dt argument is null");

    timer->__last               = timer->__now;
    timer->__now                = SDL_GetPerformanceCounter();
    u64 counter_elapsed         = timer->__now - timer->__last;
    timer->elapsed_in_ms        = 1000.0f * (f64)counter_elapsed / SDL_GetPerformanceFrequency();

    timer->__delta_accumulator += (timer->elapsed_in_ms / 1000.0f);
    timer->__frame_counter++;
    
    // Every 10th frame is when the frame rate is calculated
    if (timer->__frame_counter == 10) {

        f64 avg_delta = timer->__delta_accumulator / 10.0f;
        timer->fps_value = 1.0f / avg_delta;
        timer->__frame_counter = 0;
        timer->__delta_accumulator = 0.0f;
    }

    return true;
}




