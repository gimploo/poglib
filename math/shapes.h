#ifndef __MY__SHAPES__H__
#define __MY__SHAPES__H__

#include "la.h"

typedef struct trif {
    vec2f cmp[3];
} trif_t;

#define QUAD_FMT    VEC2F_FMT ",\n" VEC2F_FMT ",\n" VEC2F_FMT ",\n" VEC2F_FMT "\n" 
#define QUAD_ARG(q) VEC2F_ARG(q.cmp[0]), VEC2F_ARG(q.cmp[1]), VEC2F_ARG(q.cmp[2]), VEC2F_ARG(q.cmp[3]) 

typedef struct quadf {
    vec2f cmp[4];
} quadf_t;



quadf_t quadf_init(vec2f position, f32 width, f32 height)
{
    quadf_t output;
    output.cmp[0] = position;
    output.cmp[1].cmp[X] = position.cmp[X] + width;
    output.cmp[1].cmp[Y] = position.cmp[Y];

    output.cmp[2].cmp[X] = position.cmp[X] + width;
    output.cmp[2].cmp[Y] = position.cmp[Y] - height;

    output.cmp[3].cmp[X] = position.cmp[X];
    output.cmp[3].cmp[Y] = position.cmp[Y] - height;


    return output;
}

void quadf_translate(quadf_t *quad, vec2f vec)
{
    quad->cmp[0] = vec2f_add(quad->cmp[0] , vec);
    quad->cmp[1] = vec2f_add(quad->cmp[1] , vec);
    quad->cmp[2] = vec2f_add(quad->cmp[2] , vec);
    quad->cmp[3] = vec2f_add(quad->cmp[3] , vec);
}


#endif //__MY__SHAPES__H__
