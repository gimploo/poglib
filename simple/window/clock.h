#pragma once

#include <SDL2/SDL.h>
#include "../../basic.h"
#include <math.h>
#include <limits.h>

//FIXME: the fps is twice the actual value and i am not sure whether its the elapsed_in_ms
// calculations fault or not, probably it might be. 

typedef struct window_clock_t {

    f64 __now;
    f64 __last;
    u32 __frame_counter;
    f64 __accumulator;

    f64 dt_value; // delta time in seconds
    f64 fps_value;

} window_clock_t;

#define __hiretime_in_seconds() SDL_GetTicks() * 0.001f


window_clock_t window_clock_init(void)
{
    window_clock_t output = {0};
    output.__now = __hiretime_in_seconds();
    output.dt_value = 0.01f;
    return output;
}

bool window_clock_update(window_clock_t *timer)
{
    if (timer == NULL) eprint("dt argument is null");

    timer->__last               = timer->__now;
    timer->__now                = __hiretime_in_seconds();
    f64 counter_elapsed         = timer->__now - timer->__last;

    if (counter_elapsed > 0.25f) counter_elapsed = 0.25f;

    timer->__accumulator += counter_elapsed;
    timer->__frame_counter++;

    //NOTE: not finished -> https://www.youtube.com/watch?v=RaB60Ujle7o&t=807s&ab_channel=codergopher
    
    // Every 10th frame is when the frame rate is calculated
    if (timer->__frame_counter == 15) {

        f64 avg_delta = timer->__accumulator / 10.0f;
        timer->fps_value = 1.0f / avg_delta;
        timer->__frame_counter = 0;
        timer->__accumulator = 0.0f;
    }

    return true;
}




