#pragma once
#include <poglib/basic.h>
#include "la.h"

#define TOP_LEFT     0
#define TOP_RIGHT    1
#define BOTTOM_RIGHT 2
#define BOTTOM_LEFT  3

// Triangle
typedef struct trif_t {
    vec3f_t vertex[3]; 
} trif_t ; 

trif_t          trif(vec3f_t pos, f32 side);

#define         TRI_FMT         VEC3F_FMT ",\n" VEC3F_FMT ",\n" VEC3F_FMT ",\n"
#define         TRI_ARG(TRI)    VEC3F_ARG(&(TRI.vertex[0])), VEC3F_ARG(&(TRI.vertex[1])), VEC3F_ARG(&(TRI.vertex[3]))

typedef struct rect_t { 
    vec2f_t vertex[4]; 
} rect_t ;


//Quad
typedef struct quadf_t { 
    vec3f_t vertex[4]; 
} quadf_t ;

quadf_t quadf_cast(const rect_t self)
{
    const f32 fill = 0.f;
    return (quadf_t) {
        .vertex = {
            [0] = { self.vertex[0].x, self.vertex[0].y, fill },
            [1] = { self.vertex[1].x, self.vertex[1].y, fill },
            [2] = { self.vertex[2].x, self.vertex[2].y, fill },
            [3] = { self.vertex[3].x, self.vertex[3].y, fill },
        }
    };
}

#define         QUAD_FMT                        VEC3F_FMT ",\n" VEC3F_FMT ",\n" VEC3F_FMT ",\n" VEC3F_FMT "\n" 
#define         QUAD_ARG(QUAD)                  VEC3F_ARG(&(QUAD.vertex[0])), VEC3F_ARG(&(QUAD.vertex[1])), VEC3F_ARG(&(QUAD.vertex[3])), VEC3F_ARG(&(QUAD.vertex[3]))

quadf_t         quadf(vec3f_t position, f32 width, f32 height);
quadf_t         quadf_sub(quadf_t a, quadf_t b);
void            quadf_translate(quadf_t *quad, vec3f_t vec);
void            quadf_scale(quadf_t *quad, f32 scale);
bool            quadf_is_point_in_quad(const quadf_t quad, const vec2f_t point);
void            quadf_print(quadf_t quad);

#define         quadf_get_width(PQUADF)         abs((PQUADF)->vertex[1].x - (PQUADF)->vertex[3].x)
#define         quadf_get_height(PQUADF)        abs((PQUADF)->vertex[1].y - (PQUADF)->vertex[3].y)
#define         quadf_copy(DESTINATION, SRC)    memcpy(DESTINATION, SRC, sizeof(quadf_t))

//Circle
#define MAX_VERTICES_PER_CIRCLE     60 
#define MAX_TRIANGLES_PER_CIRCLE    (MAX_VERTICES_PER_CIRCLE / 3)

typedef struct circle_t {

    vec3f_t points[MAX_VERTICES_PER_CIRCLE];

} circle_t ;

circle_t        circle(vec3f_t pos, f32 radius);

//Polygon
static_assert(MAX_VERTICES_PER_CIRCLE <= 255, "EXCEEDED U8 MAX");

typedef struct polygon_t { 

    circle_t vertices;
    u8       sides;

} polygon_t ;

polygon_t       polygon(const vec3f_t pos, const f32 radius, const u8 nsides);


/*-----------------------------------------------------------------------------
                        -- IMPLEMENTATION --
-----------------------------------------------------------------------------*/
#ifndef IGNORE_SHAPES_IMPLEMENTATION

trif_t trif(vec3f_t pos, f32 side)
{
    const f32 side_half = side / 2;
    const f32 height = 1.732050807568877f * side_half; 

    return (trif_t ) {
        .vertex[0] = { pos.x, pos.y, pos.z },
        .vertex[1] = { pos.x - side_half, pos.y - height, pos.z },
        .vertex[2] = { pos.x + side_half, pos.y - height, pos.z }
    };
}

#define VEC2F_FMT "%f %f"
#define VEC2F_ARG(VEC) (VEC).raw[0], (VEC).raw[1]

#define VEC3F_FMT "%f %f %f"
#define VEC3F_ARG(VEC) (VEC).raw[0], (VEC).raw[1], (VEC).raw[2]

#define VEC4F_FMT "%f %f %f %f"
#define VEC4F_ARG(VEC) (VEC).raw[0], (VEC).raw[1], (VEC).raw[2], (VEC).raw[3]

#define VEC4I_FMT "%i %i %i %i"
#define VEC4I_ARG(VEC) (VEC).raw[0], (VEC).raw[1], (VEC).raw[2], (VEC).raw[3]

void quadf_print(quadf_t quad)
{
    printf(VEC3F_FMT", ", VEC3F_ARG(quad.vertex[0]));
    printf(VEC3F_FMT", \n", VEC3F_ARG(quad.vertex[1]));
    printf(VEC3F_FMT", ", VEC3F_ARG(quad.vertex[2]));
    printf(VEC3F_FMT", \n", VEC3F_ARG(quad.vertex[3]));
}

quadf_t quadf(vec3f_t position, f32 width, f32 height)
{
    quadf_t output = {0};

    output.vertex[0] = position;

    output.vertex[1].x = position.x + width;
    output.vertex[1].y = position.y;
    output.vertex[1].z = position.z;

    output.vertex[2].x = position.x + width;
    output.vertex[2].y = position.y - height;
    output.vertex[2].z = position.z;

    output.vertex[3].x = position.x;
    output.vertex[3].y = position.y - height;
    output.vertex[3].z = position.z;

    return output;
}

quadf_t quadf_for_window_coordinates(
    const vec3f_t pos, 
    const f32 width, 
    const f32 height
) {
    return (quadf_t) {
        .vertex = {
            [TOP_LEFT] = (vec3f_t) { 
                pos.x, 
                pos.y, 
                pos.z
            },
            [TOP_RIGHT] = (vec3f_t) { 
                pos.x + width, 
                pos.y, 
                pos.z
            },
            [BOTTOM_LEFT] = (vec3f_t) { 
                pos.x, 
                pos.y + height, 
                pos.z
            },
            [BOTTOM_RIGHT] = (vec3f_t) { 
                pos.x + width, 
                pos.y + height, 
                pos.z
            }
        }
    };
}

void quadf_translate(quadf_t *quad, vec3f_t vec)
{
    quad->vertex[0] = glms_vec3_add(quad->vertex[0],vec);
    quad->vertex[1] = glms_vec3_add(quad->vertex[1],vec);
    quad->vertex[2] = glms_vec3_add(quad->vertex[2],vec);
    quad->vertex[3] = glms_vec3_add(quad->vertex[3],vec);
}

quadf_t quadf_sub(quadf_t a, quadf_t b)
{
    quadf_t quad = {0};
    quad.vertex[0] = glms_vec3_sub(a.vertex[0] , b.vertex[0]);
    quad.vertex[1] = glms_vec3_sub(a.vertex[1] , b.vertex[1]);
    quad.vertex[2] = glms_vec3_sub(a.vertex[2] , b.vertex[2]);
    quad.vertex[3] = glms_vec3_sub(a.vertex[3] , b.vertex[3]);

    return quad;
}

void quadf_scale(quadf_t *quad, f32 scale)
{
    quad->vertex[0] = glms_vec3_scale(quad->vertex[0], scale);
    quad->vertex[1] = glms_vec3_scale(quad->vertex[1], scale);
    quad->vertex[2] = glms_vec3_scale(quad->vertex[2], scale);
    quad->vertex[3] = glms_vec3_scale(quad->vertex[3], scale);
}

bool quadf_is_point_in_quad(const quadf_t quad, const vec2f_t point)
{
    return (quad.vertex[TOP_LEFT].x < point.x 
            && quad.vertex[TOP_RIGHT].x > point.x 
            && quad.vertex[TOP_LEFT].y > point.y
            && quad.vertex[BOTTOM_LEFT].y <  point.y);
}

circle_t circle(vec3f_t pos, f32 radius)
{
    circle_t output;

    const f32 twicepi = 2.0f * (f32)PI;

    output.points[0] = pos;
    for (u64 i = 1; i < MAX_VERTICES_PER_CIRCLE; i++)
    {
        f32 angle = i * twicepi / MAX_TRIANGLES_PER_CIRCLE;

        vec3f_t point = {
            .x = pos.x + (f32)cos(angle) * radius,
            .y = pos.y + (f32)sin(angle) * radius,
            .z = pos.z
        };
        output.points[i] = point;
    }
    return output;
}

polygon_t polygon(const vec3f_t pos, const f32 radius, const u8 nsides)
{
    assert(nsides * 3 <= MAX_VERTICES_PER_CIRCLE);
    circle_t output;

    const f32 twicepi = 2.0f * (f32)PI;

    output.points[0] = pos;
    for (u64 i = 1; i < (nsides * 3); i++)
    {
        f32 angle = i * twicepi / nsides;

        vec3f_t point = {
            .x = pos.x + (f32)cos(angle) * radius,
            .y = pos.y + (f32)sin(angle) * radius,
            .z = pos.z
        };
        output.points[i] = point;
    }
    const u8 nvertices = nsides * 3;
    return (polygon_t ) {
        .vertices       = output,
        .sides          = nsides,
    };
}


f32 circle_get_radius(circle_t *circle)
{
    assert(circle);

    return (circle->points[1].x - circle->points[0].x)/ (f32) cos((2.0f * PI) / MAX_TRIANGLES_PER_CIRCLE) ;
}
#endif
