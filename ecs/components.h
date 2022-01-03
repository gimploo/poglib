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

typedef struct entitycomponent_t entitycomponent_t ;

typedef enum entitycomponent_type {

    ECT_c_boxcollider2d_t , 
    ECT_c_lifespan_t,
    ECT_c_shape2d_t,
    ECT_c_sprite_t,
    ECT_c_transform_t,
    ECT_c_shader_t,
    ECT_c_texture2d_t,
    ECT_c_input_t,

    // Add new component types here

    ECT_COUNT

} entitycomponent_type;
