#pragma once
#include "../../simple/window.h"


typedef struct c_input_t c_input_t;
c_input_t * c_input_init(window_t *win);


#ifndef IGNORE_C_INPUT_IMPLEMENTATION

struct c_input_t {

    window_t *win;
};

c_input_t * c_input_init(window_t * win)
{
    c_input_t *o = (c_input_t *)calloc(1, sizeof(c_input_t) );
    assert(o);

    *o = (c_input_t ){
        .win = win
    };

    return o;
}

#endif
