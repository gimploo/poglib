#pragma once
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
void            glcamera_update(
                    glcamera_t *const self, 
                    const f32 z_offset,     //NOTE: this needs to account in delta time also
                    const vec2f_t euler_angle
                );
matrix4f_t      glcamera_getview(const glcamera_t *const self);


/*-----------------------------------------------------------------------------
                       -- IMPLEMENTATION --
-----------------------------------------------------------------------------*/
#ifndef IGNORE_GL_CAMERA_IMPLEMENTATION

void glcamera__internal_update_directions(glcamera_t *self, const vec2f_t delta_rot)
{
    // 1. Accumulate the frame deltas into absolute angles
    self->euler_angle.x += delta_rot.x; // Pitch
    self->euler_angle.y += delta_rot.y; // Yaw

    // 2. Correctly clamp the Pitch (X axis) using min/max
    const f32 pitch_limit = radians(89.0f);
    self->euler_angle.x = fmaxf(-pitch_limit, fminf(self->euler_angle.x, pitch_limit));

    // 3. Keep Yaw (Y axis) wrapped inside 0 to 2PI range to prevent variable overflow
    self->euler_angle.y = fmodf(self->euler_angle.y, 2.0f * PI);

    // 4. Calculate vectors cleanly from absolute angles (eliminates drift/stutter)
    vec3f_t new_front;
    new_front.x = cosf(self->euler_angle.y) * cosf(self->euler_angle.x);
    new_front.y = sinf(self->euler_angle.x);
    new_front.z = sinf(self->euler_angle.y) * cosf(self->euler_angle.x);

    self->direction.front = glms_normalize(new_front);
    self->direction.right = glms_normalize(glms_cross(self->direction.front, GL_CAMERA_DIRECTION_UP));
    self->direction.up    = glms_cross(self->direction.right, self->direction.front);
}


void glcamera_update(
    glcamera_t *const self, 
    const f32 z_offset,         //NOTE: this needs to account in delta time also
    const vec2f_t delta_rot
) {
    self->position = glms_vec3_add(
        self->position, 
        glms_vec3_scale(
            self->direction.front, 
            z_offset
        )
    );

    glcamera__internal_update_directions(self, delta_rot);

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

    glcamera__internal_update_directions(&o, radians);

    logging("[CAMERA] left click look around and wasd to move the camera\n");
    return o;
}

#endif
