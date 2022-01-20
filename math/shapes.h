#pragma once
#include "vec.h"

// Triangle
typedef struct trif_t trif_t;

trif_t          trif_init(vec3f_t pos, f32 side);

#define         TRI_FMT         VEC3F_FMT ",\n" VEC3F_FMT ",\n" VEC3F_FMT ",\n"
#define         TRI_ARG(TRI)    VEC3F_ARG(&(TRI.vertex[0])), VEC3F_ARG(&(TRI.vertex[1])), VEC3F_ARG(&(TRI.vertex[3]))



//Quad
typedef struct quadf_t quadf_t;

#define         QUAD_FMT                        VEC3F_FMT ",\n" VEC3F_FMT ",\n" VEC3F_FMT ",\n" VEC3F_FMT "\n" 
#define         QUAD_ARG(QUAD)                  VEC3F_ARG(&(QUAD.vertex[0])), VEC3F_ARG(&(QUAD.vertex[1])), VEC3F_ARG(&(QUAD.vertex[3])), VEC3F_ARG(&(QUAD.vertex[3]))

quadf_t         quadf_init(vec3f_t position, f32 width, f32 height);
void            quadf_translate(quadf_t *quad, vec3f_t vec);
void            quadf_scale(quadf_t *quad, f32 scale);
bool            quadf_is_point_in_quad(quadf_t quad, vec3f_t point);
void            quadf_print(quadf_t quad);

#define         quadf_get_width(PQUADF)         abs((PQUADF)->vertex[1].cmp[X] - (PQUADF)->vertex[3].cmp[X])
#define         quadf_get_height(PQUADF)        abs((PQUADF)->vertex[1].cmp[Y] - (PQUADF)->vertex[3].cmp[Y])
#define         quadf_copy(DESTINATION, SRC)    memcpy(DESTINATION, SRC, sizeof(quadf_t))



//Circle
typedef struct circle_t circle_t;

circle_t        circle_init(vec3f_t pos, f32 radius);





#ifndef IGNORE_SHAPES_IMPLEMENTATION

#define TOP_LEFT     0
#define TOP_RIGHT    1
#define BOTTOM_RIGHT 2
#define BOTTOM_LEFT  3

#define MAX_VERTICES_PER_CIRCLE     60
#define MAX_TRIANGLES_PER_CIRCLE    (MAX_VERTICES_PER_CIRCLE / 3)

struct trif_t { 

    vec3f_t vertex[3]; 
}; 


trif_t trif_init(vec3f_t pos, f32 side)
{
    const f32 side_half = side / 2;
    const f32 height = 1.732050807568877f * side_half; 

    return (trif_t ) {
        .vertex[0] = pos,
        .vertex[1] = { pos.cmp[X] - side_half, pos.cmp[Y] - height, pos.cmp[Z] },
        .vertex[2] = { pos.cmp[X] + side_half, pos.cmp[Y] - height, pos.cmp[Z] }
    };
}



struct quadf_t { 

    vec3f_t vertex[4]; 
};


void quadf_print(quadf_t quad)
{
    for (int i = 0; i < 4; i++)
    {
        printf(VEC3F_FMT", ", VEC3F_ARG(&quad.vertex[0]));
        printf(VEC3F_FMT", \n", VEC3F_ARG(&quad.vertex[1]));
        printf(VEC3F_FMT", ", VEC3F_ARG(&quad.vertex[3]));
        printf(VEC3F_FMT", \n", VEC3F_ARG(&quad.vertex[3]));
    }
    printf("\n");
}



quadf_t quadf_init(vec3f_t position, f32 width, f32 height)
{
    quadf_t output = {0};

    output.vertex[0] = position;
    output.vertex[1].cmp[X] = position.cmp[X] + width;
    output.vertex[1].cmp[Y] = position.cmp[Y];
    output.vertex[2].cmp[X] = position.cmp[X] + width;
    output.vertex[2].cmp[Y] = position.cmp[Y] - height;
    output.vertex[3].cmp[X] = position.cmp[X];
    output.vertex[3].cmp[Y] = position.cmp[Y] - height;

    return output;
}

void quadf_translate(quadf_t *quad, vec3f_t vec)
{
    quad->vertex[0] = vec3f_add(quad->vertex[0] , vec);
    quad->vertex[1] = vec3f_add(quad->vertex[1] , vec);
    quad->vertex[2] = vec3f_add(quad->vertex[2] , vec);
    quad->vertex[3] = vec3f_add(quad->vertex[3] , vec);
}

void quadf_scale(quadf_t *quad, f32 scale)
{
    quad->vertex[0] = vec3f_scale(quad->vertex[0], scale);
    quad->vertex[1] = vec3f_scale(quad->vertex[1], scale);
    quad->vertex[2] = vec3f_scale(quad->vertex[2], scale);
    quad->vertex[3] = vec3f_scale(quad->vertex[3], scale);
}

bool quadf_is_point_in_quad(quadf_t quad, vec3f_t point)
{
    return (quad.vertex[TOP_LEFT].cmp[X] < point.cmp[X] 
            && quad.vertex[TOP_RIGHT].cmp[X] > point.cmp[X] 
            && quad.vertex[TOP_LEFT].cmp[Y] > point.cmp[Y]
            && quad.vertex[BOTTOM_LEFT].cmp[Y] <  point.cmp[Y]);
}



struct circle_t {

    vec3f_t     points[MAX_VERTICES_PER_CIRCLE];

};

circle_t circle_init(vec3f_t pos, f32 radius)
{
    circle_t output;

    const f32 twicepi = 2.0f * (f32)PI;

    output.points[0] = pos;
    for (u64 i = 1; i < MAX_VERTICES_PER_CIRCLE; i++)
    {
        if (i%3 == 0) {
            output.points[i] = pos;
            continue;
        }

        f32 angle = i * twicepi / MAX_TRIANGLES_PER_CIRCLE;

        vec3f_t point = {
            .cmp[X] = pos.cmp[X] + (f32)cos(angle) * radius,
            .cmp[Y] = pos.cmp[Y] + (f32)sin(angle) * radius,
            .cmp[Z] = pos.cmp[Z]
        };
        output.points[i] = point; 


    }
    return output;
}

f32 circle_get_radius(circle_t *circle)
{
    assert(circle);

    return (circle->points[1].cmp[X] - circle->points[0].cmp[X])/ (f32) cos((2.0f * PI) / MAX_TRIANGLES_PER_CIRCLE) ;
}
#endif
