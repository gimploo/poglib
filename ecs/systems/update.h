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

    vec3f_t topright = vec3f_add(
            t->position, 
            (vec3f_t ){ -shape->radius/2, shape->radius/2, 0.0f });

    switch(shape->type)
    {
        //Triangle
        case CST_TRIANGLE: {

            trif_t tri = trif_init(topright, shape->radius);
            mesh->model = matrixf_init(tri.vertex, 3, 3);

        } break;

        // Square
        case CST_SQUARE: {

            quadf_t tmp = quadf_init(topright, shape->radius, shape->radius);
            mesh->model = matrixf_init(tmp.vertex, 4, 3);

        } break;

        //Circle
        case CST_CIRCLE: {
            
            circle_t circle = circle_init(t->position, shape->radius);
            mesh->model = matrixf_init(circle.points, MAX_VERTICES_PER_CIRCLE, 3);

        } break;

        default: eprint("side unaccounted for");
    }

}

void transform_mesh2d_update(c_transform_t *t, c_mesh2d_t *mesh)
{
    assert(t);
    assert(mesh);

    f32 tmp[MAX_VERTICES_PER_CIRCLE]; 
    for (u64 i = 0; i < MAX_VERTICES_PER_CIRCLE; i++) tmp[i] = 1.0f;

    // TRANSFORMING VERTICES FROM OPENGL TO A MATH ONLY MESH MODEL
    matrixf_t model = matrixf_transpose(mesh->model);
    matrixf_add_row(&model, tmp);

    // ROTATION
    if (t->angular_speed != 0) {

        f32 arr[] = {

                1.0f, 0.0f, 0.0f, t->position.cmp[X],
                0.0f, 1.0f, 0.0f, t->position.cmp[Y],
                0.0f, 0.0f, 1.0f, t->position.cmp[Z],
                0.0f, 0.0f, 0.0f, 1.0f,
        };
        matrixf_t origin_shift  = matrix4f_init((vec4f_t *)arr);
        matrixf_t rot           = matrix4f_rotation_init(t->angular_radians);
        matrixf_t rtransform    = matrixf_product(rot, origin_shift);

        model = matrixf_product(rtransform, model);

    } 

    // TRANSLATION
    matrixf_t trans = matrix4f_translation_init(t->velocity);
    model = matrixf_product(
                trans,
                model);

    // Ignoring the added row of 1s for the math model
    model.nrow--;

    // TRANSFORMING BACK TO THE OPENGL MODEL FROM THE MATH MODEL
    mesh->model = matrixf_transpose(model);
}


void transform_boxcollider2d_update(c_transform_t *t, c_boxcollider2d_t *box)
{
    box->position = t->position;
    c_boxcollider2d_update(box);
}

