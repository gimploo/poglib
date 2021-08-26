#ifndef __GAME_LOGIC_DELTA_TIME_H__
#define __GAME_LOGIC_DELTA_TIME_H__

#include <SDL2/SDL.h>
#include "../../basic.h"

typedef struct game_loop_time_t {

    f64 elapsed_in_ms;

    u64 dt_now;
    u64 dt_last;
    f64 dt_value;

    u64 fps_now;
    u64 fps_last;
    f64 fps_value;

    bool is_second_iteration;
 
} game_loop_time_t;


#define __calculate_elapsed_time_in_ms(now, last)   ((last - now)/(double )SDL_GetPerformanceFrequency() * 1000.0f)
#define __calculate_delta_time(now, last)           (double)(((now) - (last))*1000 / (double )SDL_GetPerformanceFrequency())
#define __calculate_fps(now, last)                  (1.0f/((double)(((now) - (last)) / (double )SDL_GetPerformanceFrequency())))

game_loop_time_t game_loop_time_init(void)
{
    return (game_loop_time_t) {0};
}

bool game_loop_time_calculate(game_loop_time_t *clock)
{
    if (clock == NULL) eprint("dt argument is null");

    clock->dt_last = clock->dt_now;
    clock->dt_now = SDL_GetPerformanceCounter();
    clock->dt_value = __calculate_delta_time(clock->dt_now, clock->dt_last);

    clock->fps_last = clock->fps_now;
    clock->fps_now = clock->dt_now;
    clock->fps_value = __calculate_fps(clock->fps_now, clock->fps_last); 

    if (clock->is_second_iteration)
        clock->elapsed_in_ms = __calculate_elapsed_time_in_ms(clock->fps_last, clock->fps_now);
    else 
        clock->elapsed_in_ms = 16.666f;

    clock->is_second_iteration = true;

    return true;
}

#endif //__GAME_LOGIC_DELTA_TIME_H__
