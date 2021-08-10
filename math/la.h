#ifndef __MY_LA_H__
#define __MY_LA_H__

#include "../basic.h"
#include <math.h>

#define norm(x, min, max) (((x) - (min))/((max) - (min)))

#define X 0
#define Y 1
#define Z 2

typedef struct vec2 vec2;
typedef struct vec3 vec3;
typedef struct vec4 vec4;

struct vec2 { 
    u8 cmp[2];
}; 
struct vec3 { 
    u8 cmp[3];
};
struct vec4 { 
    u8 cmp[4];
};


#define VEC2F_FMT       "(%f,%f)"
#define VEC2F_ARG(vec)  (vec).cmp[0], (vec).cmp[1]
typedef struct vec2f vec2f;
typedef struct vec3f vec3f;
typedef struct vec4f vec4f;

struct vec2f {
    f32 cmp[2];
};
struct vec3f {
    f32 cmp[3];
};
struct vec4f {
    f32 cmp[4];
};


#define vec2f_translate(x, y) vec2f_add(x, y)
vec2f vec2f_add(vec2f x, vec2f y)
{
    vec2f output;
    output.cmp[X] = x.cmp[X] + y.cmp[X];
    output.cmp[Y] = x.cmp[Y] + y.cmp[Y];

    return output;
}


void matrix4f_copy(vec2f destination[], vec2f src[])
{
    memcpy(destination, src, sizeof(vec2f) * 4);
}

#endif //__MY_LA_H__
