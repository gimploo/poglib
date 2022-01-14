#pragma once
#include "../components.h"


void transform_update(c_transform_t *t)
{
    t->update(t);
}

void transform_c_shape2d_mesh2d_update(c_transform_t *t, c_shape2d_t *shape, c_mesh2d_t *mesh)
{    
    assert(t);
    assert(shape);
    assert(mesh);

    assert(mesh->vertex_count == 0);

    vec3f_t topright = vec3f_add(
            t->position, 
            (vec3f_t ){ -shape->radius/2, shape->radius/2, 0.0f });

    switch(shape->type)
    {
        //Triangle
        case CST_TRIANGLE: {

            trif_t tri = trif_init(topright, shape->radius);
            memcpy(mesh->vertices, &tri, sizeof(tri));
            mesh->vertex_count = 3;

        } break;

        // Square
        case CST_SQUARE: {

            quadf_t tmp = quadf_init(topright, shape->radius, shape->radius);
            memcpy(mesh->vertices, &tmp, sizeof(tmp));
            mesh->vertex_count = 4;

        } break;

        //Circle
        case CST_CIRCLE: {
            
            circle_t circle = circle_init(t->position, shape->radius);
            memcpy(mesh->vertices, circle.points, sizeof(circle.points));
            mesh->vertex_count = MAX_VERTICES_PER_CIRCLE;

        } break;

        default: eprint("side unaccounted for");
    }

}

void transform_mesh2d_update(c_transform_t *t, c_mesh2d_t *mesh)
{
    assert(t);
    assert(mesh);

    // Translation
    vec3f_t *arr = (vec3f_t *)mesh->vertices;
    for (u32 i = 0; i < mesh->vertex_count; i++)
    {
        arr[i] = vec3f_add(arr[i], t->velocity);
    }

    //quadf_print(*(quadf_t *)mesh->vertices);

    //if (transform->angular_speed != 0)
    //{
        //static f32 angle_in_radians = 0.0f;
        //f32 delta = transform->angular_radians;

        //angle_in_radians += delta;
        //if ((angle_in_radians >= 1.5708f) || (angle_in_radians <= -1.5708f)) {
            //delta *= -1.0f;
        //}

        //matrix4f_t rotation = matrix4f_rotation_init(angle_in_radians);

    //}
}


void transform_boxcollider2d_update(c_transform_t *t, c_boxcollider2d_t *box)
{
    box->position = t->position;
    c_boxcollider2d_update(box);
}

