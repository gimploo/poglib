#pragma once
#include "../../simple/gl/types.h"
#include "../components/shape.h"


//TODO: mesh seems complicated for just 2d things, keeping for 3d later 


typedef struct c_mesh2d_t c_mesh2d_t ;

c_mesh2d_t * c_mesh2d_init(vec3f_t centerpos, c_shape_type type, f32 radius);



#ifndef IGNORE_C_MESH_IMPLEMENTATION

struct c_mesh2d_t {

    matrixf_t model;
};

c_mesh2d_t * c_mesh2d_init(vec3f_t centerpos, c_shape_type type, f32 radius)
{
    c_mesh2d_t *o = (c_mesh2d_t *)calloc(1, sizeof(c_mesh2d_t ));
    assert(o);

    vec3f_t topright = vec3f_add(
            centerpos, 
            (vec3f_t ){ -radius/2, radius/2, 0.0f });

    switch(type)
    {
        //Triangle
        case TRIANGLE: {

            trif_t tri = trif(topright, radius);
            o->model = matrixf(tri.vertex, 3, 3);

        } break;

        // Square
        case SQUARE: {

            quadf_t tmp = quadf(topright, radius, radius);
            o->model = matrixf(tmp.vertex, 4, 3);

        } break;

        //Circle
        case CIRCLE: {
            
            circle_t tmp = circle(centerpos, radius);
            o->model = matrixf(tmp.points, MAX_VERTICES_PER_CIRCLE, 3);

        } break;

        default: eprint("side unaccounted for");
    }

    return o;
}





#endif
