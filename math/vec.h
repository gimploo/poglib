#ifndef __MY_LA_H__
#define __MY_LA_H__

#include <poglib/basic.h>
#include <math.h>


// Fast inverse square root from Quake
float Q_rsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

#define normalize(x, min, max)   (((x) - (min))/((max) - (min)))
#define denormalize(x, min, max) ((x) * ((max) - (min)) + (min))

//From a random on discord
#define map(value, from_low, from_high, to_low, to_high) ((value - from_low) * (to_high - to_low) / (from_high - from_low) + to_low)

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

typedef struct vec2ui_t vec2ui_t;
typedef struct vec3ui_t vec3ui_t;
typedef struct vec4ui_t vec4ui_t;

struct vec2ui_t { 
    u32 cmp[2];
}; 
struct vec3ui_t { 
    u32 cmp[3];
};
struct vec4ui_t { 
    u32 cmp[4];
};
#define VEC2UI_FMT       "(%i,%i)"
#define VEC2UI_ARG(pvec)  (pvec)->cmp[0], (pvec)->cmp[1]
#define VEC3UI_FMT       "(%i,%i,%i)"
#define VEC3UI_ARG(pvec)  (pvec)->cmp[0], (pvec)->cmp[1], (pvec)->cmp[2]
#define VEC4UI_FMT       "(%i,%i,%i,%i)"
#define VEC4UI_ARG(pvec)  (pvec)->cmp[0], (pvec)->cmp[1], (pvec)->cmp[2], (pvec)->cmp[3]

typedef struct vec2i_t vec2i_t;
typedef struct vec3i_t vec3i_t;
typedef struct vec4i_t vec4i_t;

struct vec2i_t { 
    i32 cmp[2];
}; 
struct vec3i_t { 
    i32 cmp[3];
};
struct vec4i_t { 
    i32 cmp[4];
};

#define VEC2I_FMT       "(%d,%d)"
#define VEC2I_ARG(pvec)  (pvec)->cmp[0], (pvec)->cmp[1]
#define VEC3I_FMT       "(%d,%d,%d)"
#define VEC3I_ARG(pvec)  (pvec)->cmp[0], (pvec)->cmp[1], (pvec)->cmp[2]
#define VEC4I_FMT       "(%d,%d,%d,%d)"
#define VEC4I_ARG(pvec)  (pvec)->cmp[0], (pvec)->cmp[1], (pvec)->cmp[2], (pvec)->cmp[3]


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

#define vec2f(X) _Generic((X), \
\
        f32:        __vec2float,    \
        vec3f_t:    __vec2vec3f,    \
        vec4f_t:    __vec2vec4f     \
\
)(X)

vec2f_t __vec2float(f32 x)
{
    return (vec2f_t) { x, x };
}

vec2f_t __vec2vec3f(vec3f_t x)
{
    return *(vec2f_t *)&x;
}

vec2f_t __vec2vec4f(vec4f_t x)
{
    return *(vec2f_t *)&x;
}

#define vec3f(X) _Generic((X), \
\
        f32:        __vec3float,    \
        vec2f_t:    __vec3vec2f,     \
        vec4f_t:    __vec3vec4f     \
\
)(X)

vec3f_t __vec3float(float x)
{
    return (vec3f_t) { x, x, x };
}

vec3f_t __vec3vec2f(vec2f_t x)
{
    return (vec3f_t ){ x.cmp[X], x.cmp[Y], 0.0f };
}

vec3f_t __vec3vec4f(vec4f_t x)
{
    return *(vec3f_t *)&x;
}

#define vec4f(X) _Generic((X), \
\
        f32:        __vec4float,    \
        vec2f_t:    __vec4vec2f,     \
        vec3f_t:    __vec4vec3f     \
\
)(X)

vec4f_t __vec4float(float x)
{
    return (vec4f_t) { x, x, x, x };
}

vec4f_t __vec4vec2f(vec2f_t x)
{
    return (vec4f_t ) { x.cmp[X], x.cmp[Y], 0.0f, 0.0f };
}

vec4f_t __vec4vec3f(vec3f_t x)
{
    return (vec4f_t ) { x.cmp[X], x.cmp[Y], x.cmp[Z], 0.0f };
}


#define vec2f_copy(pdest, psrc) memcpy(pdest, psrc, sizeof(vec2f_t))
#define vec3f_copy(pdest, psrc) memcpy(pdest, psrc, sizeof(vec3f_t))
#define vec4f_copy(pdest, psrc) memcpy(pdest, psrc, sizeof(vec4f_t))


#define vec2f_translate(x, y) vec2f_add(x, y)
#define vec3f_translate(x, y) vec3f_add(x, y)
#define vec4f_translate(x, y) vec4f_add(x, y)

vec3f_t vec3f_cross(const vec3f_t x, const vec3f_t y);

vec2f_t vec2f_add(const vec2f_t x, const vec2f_t y)
{
    vec2f_t output;
    output.cmp[X] = x.cmp[X] + y.cmp[X];
    output.cmp[Y] = x.cmp[Y] + y.cmp[Y];

    return output;
}

vec2f_t vec2f_sub(const vec2f_t x, const vec2f_t y)
{
    vec2f_t output;
    output.cmp[X] = x.cmp[X] - y.cmp[X];
    output.cmp[Y] = x.cmp[Y] - y.cmp[Y];

    return output;
}

vec2f_t vec2f_scale(const vec2f_t x, const f32 scale)
{
    vec2f_t output;
    output.cmp[X] = x.cmp[X] * scale;
    output.cmp[Y] = x.cmp[Y] * scale;
    return output;
}

bool vec2f_isequal(const vec2f_t x, const vec2f_t y)
{
    return (x.cmp[X] == y.cmp[X] && x.cmp[Y] == y.cmp[Y]);
}

f32 vec2f_distance(const vec2f_t x, const vec2f_t y)
{
    return Q_rsqrt(pow((x.cmp[X] - y.cmp[X]), 2.0f) + pow((x.cmp[Y] - y.cmp[Y]), 2.0f));
}

vec3f_t vec3f_scale(const vec3f_t x, const f32 scale)
{
    vec3f_t output;
    output.cmp[X] = x.cmp[X] * scale;
    output.cmp[Y] = x.cmp[Y] * scale;
    output.cmp[Z] = x.cmp[Z] * scale;

    return output;
}

vec3f_t vec3f_radians(f32 scale, f32 angle_in_radians)
{
    assert(angle_in_radians <= 6.283185 || angle_in_radians >= -6.283185);

    return (vec3f_t ) {
        .cmp = {
            [X] = scale * (f32)cos(angle_in_radians),
            [Y] = scale * (f32)sin(angle_in_radians),
        }
    };
}

bool vec3f_isequal(const vec3f_t x, const vec3f_t y)
{
    return (x.cmp[X] == y.cmp[X] && x.cmp[Y] == y.cmp[Y] && x.cmp[Z] == y.cmp[Z]);
}

f32 vec3f_distance(const vec3f_t x, const vec3f_t y)
{
    return Q_rsqrt(pow((x.cmp[X] - y.cmp[X]), 2.0f) + pow((x.cmp[Y] - y.cmp[Y]), 2.0f) + pow((x.cmp[Z] - y.cmp[Z]), 2.0f));
}

vec3f_t vec3f_add(const vec3f_t x, const vec3f_t y)
{
    vec3f_t output;
    output.cmp[X] = x.cmp[X] + y.cmp[X];
    output.cmp[Y] = x.cmp[Y] + y.cmp[Y];
    output.cmp[Z] = x.cmp[Z] + y.cmp[Z];

    return output;
}

vec3f_t vec3f_sub(const vec3f_t x, const vec3f_t y)
{
    vec3f_t output;
    output.cmp[X] = x.cmp[X] - y.cmp[X];
    output.cmp[Y] = x.cmp[Y] - y.cmp[Y];
    output.cmp[Z] = x.cmp[Z] - y.cmp[Z];

    return output;
}

vec3f_t vec3f_cross(const vec3f_t x, const vec3f_t y)
{
    return (vec3f_t ) {
        .cmp[X] = x.cmp[Y] * y.cmp[Z] - x.cmp[Z] * y.cmp[Y],
        .cmp[Y] = x.cmp[Z] * y.cmp[X] - x.cmp[X] * y.cmp[Z],
        .cmp[Z] = x.cmp[X] * y.cmp[Y] - x.cmp[Y] * y.cmp[X],
    };
}


vec4f_t vec4f_add(const vec4f_t x, const vec4f_t y)
{
    vec4f_t output;
    output.cmp[X] = x.cmp[X] + y.cmp[X];
    output.cmp[Y] = x.cmp[Y] + y.cmp[Y];
    output.cmp[Z] = x.cmp[Z] + y.cmp[Z];
    output.cmp[W] = x.cmp[W] + y.cmp[W];

    return output;
}

vec4f_t vec4f_scale(const vec4f_t x, const f32 scale)
{
    vec4f_t output;
    output.cmp[X] = x.cmp[X] * scale;
    output.cmp[Y] = x.cmp[Y] * scale;
    output.cmp[Z] = x.cmp[Z] * scale;
    output.cmp[W] = x.cmp[W] * scale;

    return output;
}

bool vec4f_isequal(const vec4f_t x, const vec4f_t y)
{
    return (x.cmp[X] == y.cmp[X] && x.cmp[Y] == y.cmp[Y] && x.cmp[Z] == y.cmp[Z] && x.cmp[W] == y.cmp[W]);
}

f32 vec4f_distance(const vec4f_t x, const vec4f_t y)
{
    return Q_rsqrt(pow((x.cmp[X] - y.cmp[X]), 2.0f) + pow((x.cmp[Y] - y.cmp[Y]), 2.0f) + pow((x.cmp[Z] - y.cmp[Z]), 2.0f) + pow((x.cmp[W] - y.cmp[W]), 2.0f));
}

f32 vec3f_dot(const vec3f_t a, const vec3f_t b) 
{
    return a.cmp[0] * b.cmp[0] 
        + a.cmp[1] * b.cmp[1] 
        + a.cmp[2] * b.cmp[2];
}

f32 vec3f_norm(const vec3f_t v) 
{
    return sqrtf(vec3f_dot(v,v));
}

vec3f_t vec3f_normalize(const vec3f_t v) 
{
    f32 norm = vec3f_norm(v);
    if (norm == 0.0f) return vec3f(0.0f);

    return vec3f_scale(v, 1.0f / norm);
}

#endif //__MY_LA_H__
