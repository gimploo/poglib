#pragma once

#include <SDL2/SDL.h>
#include "../../basic.h"
#include <math.h>
#include <limits.h>

//FIXME: the fps is twice the actual value and i am not sure whether its the elapsed_in_ms
// calculations fault or not, probably it might be. 

typedef struct window_clock_t {

    f32 __now;
    f32 __last;
    u32 __frame_counter;
    f32 __accumulator;

    f32 dt_value; // delta time in seconds
    f32 fps_value;

} window_clock_t;


window_clock_t window_clock_init(void)
{
    window_clock_t output = {0};
    output.__now = SDL_GetTicks();
    output.dt_value = 0.01f;
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

bool window_clock_update(window_clock_t *timer)
{
    if (timer == NULL) eprint("dt argument is null");

    timer->__last   = timer->__now;
    timer->__now    = SDL_GetTicks();
    timer->dt_value = (timer->__now - timer->__last) / 100.0f;

    if (timer->dt_value > 0.25f) timer->dt_value = 0.25f;

    timer->__accumulator += timer->dt_value;
    timer->__frame_counter++;

    timer->fps_value = timer->__frame_counter / (timer->__now / 1000.0f) ;

    if(timer->__frame_counter == (UINT32_MAX - 1)) timer->__frame_counter = 0;

    return true;
}




