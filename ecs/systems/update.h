#pragma once


#include "../components/collider.h"
#include "../components/transform.h"
#include "../components/shape.h"


void transform_shape2d_update(c_transform_t *transform, c_shape2d_t *shape)
{
    shape->center_pos = transform->position;
    shape->update(shape);

    if (transform->angular_speed != 0)
    {
        shape->rotate(shape, transform->angular_speed, transform->angular_radians);
        transform->angular_radians += 0.01f;
    }
}

void transform_boxcollider2d_update(c_transform_t *t, c_boxcollider2d_t *box)
{
    box->position = t->position;
    c_boxcollider2d_update(box);
}

