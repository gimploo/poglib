#pragma once
#include "poglib/external/cglm/struct/quat.h"
#include <poglib/basic.h>
#include <poglib/math.h>

//TODO: camera doesnt account for `roll` angle (rotation around z axis)

#define GL_CAMERA_DIRECTION_FRONT    (vec3f_t ){0.0f, 0.0f, -1.0f}
#define GL_CAMERA_DIRECTION_UP       (vec3f_t ){0.0f, 1.0f, 0.0f }

const f32 GL_CAMERA_SPEED           =  1500.0f;
const f32 GL_CAMERA_SENSITIVITY     =  1.f;

typedef struct glcamera_t {

    vec3f_t     position;
    vec2f_t     euler_angle;

    struct {
        vec3f_t front;
        vec3f_t up;
        vec3f_t right;
    } direction;

} glcamera_t ;

glcamera_t      glcamera_perspective(const vec3f_t pos, const vec2f_t theta);
void            glcamera_set(
                    glcamera_t *const self, 
                    const vec3f_t absolute_position, 
                    const vec2f_t euler_angle
                );
void            glcamera_update(
                    glcamera_t *const self, 
                    const f32 z_offset,
                    const vec2f_t delta_rot
                );
matrix4f_t      glcamera_getview(const glcamera_t *const self);
void            glcamera_lookat(glcamera_t *const self, const vec3f_t target, const vec3f_t up);


/*-----------------------------------------------------------------------------
                       -- IMPLEMENTATION --
-----------------------------------------------------------------------------*/
#ifndef IGNORE_GL_CAMERA_IMPLEMENTATION

INTERNAL void glcamera__internal_update_directions(glcamera_t *const self)
{
    const vec3f_t new_front = {
        .x = cosf(self->euler_angle.y) * cosf(self->euler_angle.x),
        .y = sinf(self->euler_angle.x),
        .z = sinf(self->euler_angle.y) * cosf(self->euler_angle.x),
    };
    self->direction.front   = glms_normalize(new_front);
    self->direction.right   = glms_normalize(glms_cross(self->direction.front, GL_CAMERA_DIRECTION_UP));
    self->direction.up      = glms_cross(self->direction.right, self->direction.front);
}

void glcamera_set(
    glcamera_t *const self, 
    const vec3f_t absolute_position,         //NOTE: this needs to account in delta time also
    const vec2f_t euler_angle
) {
    self->position      = absolute_position;
    self->euler_angle   = euler_angle;

    glcamera__internal_update_directions(self);

#ifndef DISABLE_CAMERA_LOGGING
    logging("Camera Pos: "VEC3F_FMT " | " "Delta Angle: " VEC2F_FMT, 
            VEC3F_ARG(self->position), VEC2F_ARG(self->euler_angle));
#endif
}

void glcamera_update(
    glcamera_t *const self, 
    const f32 z_offset,                     //NOTE: this needs to account in delta time also
    const vec2f_t delta_rot
) {
    self->position = glms_vec3_add(
        self->position, 
        glms_vec3_scale(
            self->direction.front, 
            z_offset
        )
    );

    self->euler_angle.x += delta_rot.x; // Pitch
    self->euler_angle.y += delta_rot.y; // Yaw

    glcamera__internal_update_directions(self);

#ifndef DISABLE_CAMERA_LOGGING
    logging("Camera Pos: "VEC3F_FMT " | " "Delta Angle: " VEC2F_FMT, 
            VEC3F_ARG(self->position), VEC2F_ARG(self->euler_angle));
#endif
}

matrix4f_t glcamera_getview(const glcamera_t *const self)
{
    return glms_lookat(
        self->position, 
        glms_vec3_add(
            self->position, 
            self->direction.front
        ), 
        self->direction.up
    );
}

void glcamera_lookat(glcamera_t *const self, const vec3f_t target, const vec3f_t up)
{
    self->direction.up      = up;
    self->direction.front   = glms_normalize(glms_vec3_sub(target, self->position));
    self->direction.right   = glms_normalize(glms_cross(self->direction.front, up));
    self->euler_angle.x     = asinf(self->direction.front.y);
    self->euler_angle.y     = atan2f(self->direction.front.z, self->direction.front.x);

#if 0
    printf("cam front");    glms_vec3_print(self->direction.front,stdout);
    printf("cam up");       glms_vec3_print(self->direction.up,stdout);
    printf("cam right");    glms_vec3_print(self->direction.right,stdout);
#endif

}

glcamera_t glcamera_perspective(const vec3f_t pos, const vec2f_t radians)
{
    glcamera_t o = {
        .position   = pos,
        .euler_angle = radians,
        .direction = {
            .front      = GL_CAMERA_DIRECTION_FRONT,
            .up         = {0},
            .right      = {0},
        },
    };

    glcamera_update(&o, pos.z, radians);

    logging("[CAMERA] left click look around and wasd to move the camera\n");
    return o;
}

#endif
