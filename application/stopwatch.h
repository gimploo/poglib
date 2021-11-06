#pragma once

#include <SDL2/SDL.h>
#include "../basic.h"
#include <math.h>
#include <limits.h>

//FIXME: the fps is twice the actual value and i am not sure whether its the elapsed_in_ms
// calculations fault or not, probably it might be. 
//

typedef struct stopwatch_t stopwatch_t;

struct stopwatch_t {

    f32 __now;
    f32 __last;
    f32 accumulator;

    f32 dt; // delta time in seconds
    f32 fps;

};


stopwatch_t stopwatch_init(void)
{
    stopwatch_t output = {0};
    output.__now = (f32)SDL_GetTicks();
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
    timer->__now    = (f32)SDL_GetTicks();
    timer->dt = (timer->__now - timer->__last) / 1000.0f;

    if (timer->dt > 0.25f) timer->dt = 0.25f;

    timer->accumulator += timer->dt;

}




