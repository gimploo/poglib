#pragma once
#include "../components/shape.h"


//TODO: mesh seems complicated for just 2d things, keeping for 3d later 


typedef struct c_mesh2d_t c_mesh2d_t ;

c_mesh2d_t c_mesh2d(vec3f_t centerpos, c_shape_type type, f32 radius);



#ifndef IGNORE_C_MESH_IMPLEMENTATION

struct c_mesh2d_t {

    matrix4f_t model;
};

c_mesh2d_t c_mesh2d(vec3f_t centerpos, c_shape_type type, f32 radius)
{
    c_mesh2d_t o = {0};
    vec3f_t topright = glms_vec3_add(
            centerpos, 
            (vec3f_t ){ -radius/2, radius/2, 0.0f });

    switch(type)
    {
        //Triangle
        case TRIANGLE: {

            trif_t tri = trif(topright, radius);
            memcpy(&o.model, tri.vertex, sizeof(o.model));

        } break;

        // Square
        case SQUARE: {

            quadf_t tmp = quadf(topright, radius, radius);
            memcpy(&o.model, tmp.vertex, sizeof(o.model));

        } break;

        //Circle
        case CIRCLE: {
            
            circle_t tmp = circle(centerpos, radius);
            memcpy(&o.model, tmp.points, sizeof(o.model));

        } break;

        default: eprint("side unaccounted for");
    }

    return o;
}





#endif
