#pragma once
#include <poglib/application.h>


typedef struct c_input_t c_input_t;
c_input_t c_input(window_t *win);


#ifndef IGNORE_C_INPUT_IMPLEMENTATION

struct c_input_t {

    window_t *win;
    void (*update)(struct c_input_t *);
};

c_input_t c_input(window_t * win)
{
    return (c_input_t ){
        .win = win,
        .update = NULL 
    };
}

#endif
