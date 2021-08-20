#ifndef __MY_LA_H__
#define __MY_LA_H__

#include "../basic.h"
#include <math.h>

#define norm(x, min, max) (((x) - (min))/((max) - (min)))

#define X 0
#define Y 1
#define Z 2
#define W 3

typedef struct vec2_t vec2_t;
typedef struct vec3_t vec3_t;
typedef struct vec4_t vec4_t;

struct vec2_t { 
    u8 cmp[2];
}; 
struct vec3_t { 
    u8 cmp[3];
};
struct vec4_t { 
    u8 cmp[4];
};


#define VEC2F_FMT       "(%f,%f)"
#define VEC2F_ARG(pvec)  (pvec)->cmp[0], (pvec)->cmp[1]
#define VEC3F_FMT       "(%f,%f,%f)"
#define VEC3F_ARG(pvec)  (pvec)->cmp[0], (pvec)->cmp[1], (pvec)->cmp[2]
#define VEC4F_FMT       "(%f,%f,%f,%f)"
#define VEC4F_ARG(pvec)  (pvec)->cmp[0], (pvec)->cmp[1], (pvec)->cmp[2], (pvec)->cmp[3]
typedef struct vec2f_t vec2f_t;
typedef struct vec3f_t vec3f_t;
typedef struct vec4f_t vec4f_t;

struct vec2f_t {
    f32 cmp[2];
};
struct vec3f_t {
    f32 cmp[3];
};
struct vec4f_t {
    f32 cmp[4];
};

vec2f_t vec2f(float x)
{
    return (vec2f_t) { x, x };
}

vec3f_t vec3f(float x)
{
    return (vec3f_t) { x, x, x };
}

vec4f_t vec4f(float x)
{
    return (vec4f_t) { x, x, x, x };
}


#define vec2f_copy(pdest, psrc) memcpy(pdest, psrc, sizeof(vec2f_t))
#define vec3f_copy(pdest, psrc) memcpy(pdest, psrc, sizeof(vec3f_t))
#define vec4f_copy(pdest, psrc) memcpy(pdest, psrc, sizeof(vec4f_t))


#define vec2f_translate(x, y) vec2f_add(x, y)
vec2f_t vec2f_add(vec2f_t x, vec2f_t y)
{
    vec2f_t output;
    output.cmp[X] = x.cmp[X] + y.cmp[X];
    output.cmp[Y] = x.cmp[Y] + y.cmp[Y];

    return output;
}

vec2f_t vec2f_scale(vec2f_t x, f32 scale)
{
    vec2f_t output;
    output.cmp[X] = x.cmp[X] * scale;
    output.cmp[Y] = x.cmp[Y] * scale;
    return output;
}


void matrix4f_copy(vec2f_t destination[], vec2f_t src[])
{
    memcpy(destination, src, sizeof(vec2f_t) * 4);
}

#endif //__MY_LA_H__
