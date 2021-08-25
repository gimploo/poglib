#ifndef __MY__SHAPES__H__
#define __MY__SHAPES__H__

#include "la.h"

/*---------------------------------------------------------
 // Triangle (float type)
---------------------------------------------------------*/

typedef struct trif_t { vec2f_t vertex[3]; } trif_t; 

/*---------------------------------------------------------
 // QUAD (float type)
---------------------------------------------------------*/

typedef struct quadf_t { vec2f_t vertex[4]; } quadf_t;

#define QUAD_FMT    VEC2F_FMT ",\n" VEC2F_FMT ",\n" VEC2F_FMT ",\n" VEC2F_FMT "\n" 
#define QUAD_ARG(q) VEC2F_ARG(&(q.vertex[0])), VEC2F_ARG(&(q.vertex[1])), VEC2F_ARG(&(q.vertex[2])), VEC2F_ARG(&(q.vertex[3])) 

static inline quadf_t quadf(f32 x)
{
    return (quadf_t) {x}; 
}

#define quadf_get_width(pquadf)   abs((pquadf)->vertex[1].cmp[X] - (pquadf)->vertex[3].cmp[X])
#define quadf_get_height(pquadf)  abs((pquadf)->vertex[1].cmp[Y] - (pquadf)->vertex[3].cmp[Y])


quadf_t quadf_init(vec2f_t position, f32 width, f32 height)
{
    quadf_t output;

    output.vertex[0] = position;
    output.vertex[1].cmp[X] = position.cmp[X] + width;
    output.vertex[1].cmp[Y] = position.cmp[Y];
    output.vertex[2].cmp[X] = position.cmp[X] + width;
    output.vertex[2].cmp[Y] = position.cmp[Y] - height;
    output.vertex[3].cmp[X] = position.cmp[X];
    output.vertex[3].cmp[Y] = position.cmp[Y] - height;


    return output;
}

void quadf_translate(quadf_t *quad, vec2f_t vec)
{
    quad->vertex[0] = vec2f_add(quad->vertex[0] , vec);
    quad->vertex[1] = vec2f_add(quad->vertex[1] , vec);
    quad->vertex[2] = vec2f_add(quad->vertex[2] , vec);
    quad->vertex[3] = vec2f_add(quad->vertex[3] , vec);
}

void quadf_scale(quadf_t *quad, f32 scale)
{
    quad->vertex[0] = vec2f_scale(quad->vertex[0], scale);
    quad->vertex[1] = vec2f_scale(quad->vertex[1], scale);
    quad->vertex[2] = vec2f_scale(quad->vertex[2], scale);
    quad->vertex[3] = vec2f_scale(quad->vertex[3], scale);
}



#endif //__MY__SHAPES__H__
