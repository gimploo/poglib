#pragma once
#include "../components.h"


void transform_update(c_transform_t *t)
{
    assert(t);

    t->update(t);
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

        f32 mat01[] = {

                1.0f, 0.0f, 0.0f, -t->position.cmp[X],
                0.0f, 1.0f, 0.0f, -t->position.cmp[Y],
                0.0f, 0.0f, 1.0f, -t->position.cmp[Z],
                0.0f, 0.0f, 0.0f, 1.0f,
        };
        f32 mat02[] = {

                1.0f, 0.0f, 0.0f, t->position.cmp[X],
                0.0f, 1.0f, 0.0f, t->position.cmp[Y],
                0.0f, 0.0f, 1.0f, t->position.cmp[Z],
                0.0f, 0.0f, 0.0f, 1.0f,
        };

        matrixf_t origin_shift          = matrix4f_init((vec4f_t *)mat01);
        matrixf_t origin_shift_reset    = matrix4f_init((vec4f_t *)mat02);
        matrixf_t rot                   = matrix4f_rotation_init(t->angular_radians);

        matrixf_t rtransform = matrixf_product(
                origin_shift_reset, 
                matrixf_product(rot, origin_shift));


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
    assert(t);
    assert(box);
        
    box->centerpos = t->position;
}

