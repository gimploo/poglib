#pragma once
#include "./components/collider.h"
#include "./components/lifespan.h"
#include "./components/mesh.h"
#include "./components/shader.h"
#include "./components/shape.h"
#include "./components/sprite.h"
#include "./components/texture.h"
#include "./components/transform.h"
#include "./components/input.h"
#include "./components/state.h"

//NOTE: COMPONENT CANNOT BE 8 bytes in size, add an extra byte padding if so

typedef enum entitycomponent_type {

    ECT_c_boxcollider2d_t , 
    ECT_c_texture2d_t,
    ECT_c_shape2d_t,
    ECT_c_mesh2d_t,

    ECT_c_transform_t,
    ECT_c_lifespan_t,
    ECT_c_sprite_t,
    ECT_c_shader_t,
    ECT_c_input_t,
    ECT_c_state_t,

    // Add new component types here

    ECT_COUNT

} entitycomponent_type;

typedef struct entitycomponent_t {

    entitycomponent_type    type;

    union {
        c_transform_t       transform;
        c_lifespan_t        lifespan;
        c_sprite_t          sprite;
        c_input_t           input; 
        c_state_t           state; 

        c_boxcollider2d_t   boxcollider2d;
        c_shape2d_t         shape2d;
        c_mesh2d_t          mesh2d;

        c_texture2d_t       gltexture2d;
        c_shader_t          glshader;

    } cmp;

} entitycomponent_t ;
