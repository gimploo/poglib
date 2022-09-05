#pragma once
#include "vec.h"

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

//Quad
typedef struct quadf_t { 
    vec3f_t vertex[4]; 
} quadf_t ;

#define         QUAD_FMT                        VEC3F_FMT ",\n" VEC3F_FMT ",\n" VEC3F_FMT ",\n" VEC3F_FMT "\n" 
#define         QUAD_ARG(QUAD)                  VEC3F_ARG(&(QUAD.vertex[0])), VEC3F_ARG(&(QUAD.vertex[1])), VEC3F_ARG(&(QUAD.vertex[3])), VEC3F_ARG(&(QUAD.vertex[3]))

quadf_t         quadf(vec3f_t position, f32 width, f32 height);
quadf_t         quadf_sub(quadf_t a, quadf_t b);
void            quadf_translate(quadf_t *quad, vec3f_t vec);
void            quadf_scale(quadf_t *quad, f32 scale);
bool            quadf_is_point_in_quad(const quadf_t quad, const vec2f_t point);
void            quadf_print(quadf_t quad);

#define         quadf_get_width(PQUADF)         abs((PQUADF)->vertex[1][X] - (PQUADF)->vertex[3][X])
#define         quadf_get_height(PQUADF)        abs((PQUADF)->vertex[1][Y] - (PQUADF)->vertex[3][Y])
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
        .vertex[0] = { pos[X], pos[Y], pos[Z] },
        .vertex[1] = { pos[X] - side_half, pos[Y] - height, pos[Z] },
        .vertex[2] = { pos[X] + side_half, pos[Y] - height, pos[Z] }
    };
}
#define VEC3F_FMT "%f %f %f"
#define VEC3F_ARG(PVEC) PVEC[0], PVEC[1], PVEC[2]

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

    glm_vec3_copy(position, output.vertex[0]);

    output.vertex[1][X] = position[X] + width;
    output.vertex[1][Y] = position[Y];
    output.vertex[1][Z] = position[Z];

    output.vertex[2][X] = position[X] + width;
    output.vertex[2][Y] = position[Y] - height;
    output.vertex[2][Z] = position[Z];

    output.vertex[3][X] = position[X];
    output.vertex[3][Y] = position[Y] - height;
    output.vertex[3][Z] = position[Z];

    return output;
}

void quadf_translate(quadf_t *quad, vec3f_t vec)
{
    glm_vec3_add(quad->vertex[0],vec, quad->vertex[0]);
    glm_vec3_add(quad->vertex[1],vec,quad->vertex[1]);
    glm_vec3_add(quad->vertex[2],vec,quad->vertex[2]);
    glm_vec3_add(quad->vertex[3],vec,quad->vertex[3]);
}

quadf_t quadf_sub(quadf_t a, quadf_t b)
{
    quadf_t quad = {0};
    glm_vec3_sub(a.vertex[0] , b.vertex[0], quad.vertex[0]);
    glm_vec3_sub(a.vertex[1] , b.vertex[1], quad.vertex[1]);
    glm_vec3_sub(a.vertex[2] , b.vertex[2], quad.vertex[2]);
    glm_vec3_sub(a.vertex[3] , b.vertex[3], quad.vertex[3]);

    return quad;
}

void quadf_scale(quadf_t *quad, f32 scale)
{
    glm_vec3_scale(quad->vertex[0], scale, quad->vertex[0]);
    glm_vec3_scale(quad->vertex[1], scale, quad->vertex[1]);
    glm_vec3_scale(quad->vertex[2], scale, quad->vertex[2]);
    glm_vec3_scale(quad->vertex[3], scale, quad->vertex[3]);
}

bool quadf_is_point_in_quad(const quadf_t quad, const vec2f_t point)
{
    return (quad.vertex[TOP_LEFT][X] < point[X] 
            && quad.vertex[TOP_RIGHT][X] > point[X] 
            && quad.vertex[TOP_LEFT][Y] > point[Y]
            && quad.vertex[BOTTOM_LEFT][Y] <  point[Y]);
}

circle_t circle(vec3f_t pos, f32 radius)
{
    circle_t output;

    const f32 twicepi = 2.0f * (f32)PI;

    glm_vec3_copy(pos, output.points[0]);
    for (u64 i = 1; i < MAX_VERTICES_PER_CIRCLE; i++)
    {
        f32 angle = i * twicepi / MAX_TRIANGLES_PER_CIRCLE;

        vec3f_t point = {
            [X] = pos[X] + (f32)cos(angle) * radius,
            [Y] = pos[Y] + (f32)sin(angle) * radius,
            [Z] = pos[Z]
        };
        glm_vec3_copy(point, output.points[i]);
    }
    return output;
}

polygon_t polygon(const vec3f_t pos, const f32 radius, const u8 nsides)
{
    assert(nsides * 3 <= MAX_VERTICES_PER_CIRCLE);
    circle_t output;

    const f32 twicepi = 2.0f * (f32)PI;

    glm_vec3_copy(pos, output.points[0]);
    for (u64 i = 1; i < (nsides * 3); i++)
    {
        f32 angle = i * twicepi / nsides;

        vec3f_t point = {
            [X] = pos[X] + (f32)cos(angle) * radius,
            [Y] = pos[Y] + (f32)sin(angle) * radius,
            [Z] = pos[Z]
        };
        glm_vec3_copy(point, output.points[i]);
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

    return (circle->points[1][X] - circle->points[0][X])/ (f32) cos((2.0f * PI) / MAX_TRIANGLES_PER_CIRCLE) ;
}
#endif
