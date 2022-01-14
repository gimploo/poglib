#pragma once
#include "../../simple/gl/types.h"
#include "../components/shape.h"


//TODO: mesh seems complicated for just 2d things, keeping for 3d later 


typedef struct c_mesh2d_t c_mesh2d_t ;

c_mesh2d_t * c_mesh2d_init(void);



#ifndef IGNORE_C_MESH_IMPLEMENTATION

struct c_mesh2d_t {

    u32 vertex_count;
    u8 vertices[KB];
};

c_mesh2d_t * c_mesh2d_init(void)
{
    c_mesh2d_t *o = (c_mesh2d_t *)calloc(1, sizeof(c_mesh2d_t ));
    assert(o);

    *o = (c_mesh2d_t ) {
        .vertex_count = 0,
        .vertices = {0},
    };

    return o;
}


#endif
