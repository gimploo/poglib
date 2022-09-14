#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#define WINDOW_SDL
#include <poglib/application.h>

#define GL_CAMERA_DIRECTION_FRONT    (vec3f_t ){0.0f, 0.0f, -1.0f}
#define GL_CAMERA_DIRECTION_UP       (vec3f_t ){0.0f, 1.0f, 0.0f }

const f32 GL_CAMERA_YAW             = -90.0f;
const f32 GL_CAMERA_PITCH           =  0.0f;
const f32 GL_CAMERA_SPEED           =  2.5f;
const f32 GL_CAMERA_SENSITIVITY     =  0.1f;
const f32 GL_CAMERA_ZOOM            =  45.0f;
const f32 GL_CAMERA_ZOOM_OFFSET     =  0.2f;

typedef enum glcamera_type {

    GLCAMERATYPE_ORTHOGRAPHIC,
    GLCAMERATYPE_PERSPECTIVE,
    GLCAMREATYPE_COUNT

} glcamera_type;

typedef struct glcamera_t {

    glcamera_type   type;
    vec3f_t         position;
    vec3f_t         front;
    vec3f_t         up;
    vec3f_t         right;
    vec3f_t         worldup;
    struct {
        f32 yaw;
        f32 pitch;
    } angle;
    f32             zoom;
    matrix4f_t      __view;

} glcamera_t ;

glcamera_t      glcamera_perspective(const vec3f_t pos);
glcamera_t      glcamera_orthographic(const vec3f_t pos);
void            glcamera_process_input(glcamera_t *self, const f32 dt);
matrix4f_t      glcamera_getview(glcamera_t *self);


/*-----------------------------------------------------------------------------
                       -- IMPLEMENTATION --
-----------------------------------------------------------------------------*/
#ifndef IGNORE_GL_CAMERA_IMPLEMENTATION

void glcamera_process_input(glcamera_t *self, const f32 dt)
{
    window_t *win = window_get_current_active_window();

    const vec2f_t newmp     = window_mouse_get_norm_position(win);
    static vec2f_t oldmp    = {0.0f};

    if (window_keyboard_is_key_pressed(win, SDLK_w)) 
        self->position = glms_vec3_add(
                            self->position, 
                            glms_vec3_scale(
                                self->front, 
                                GL_CAMERA_SPEED * dt
                            )
                        );

    if (window_keyboard_is_key_pressed(win, SDLK_s))
        self->position = glms_vec3_sub(
                            self->position, 
                            glms_vec3_scale(
                                self->front, 
                                GL_CAMERA_SPEED *dt
                            )
                        );
    if (window_keyboard_is_key_pressed(win, SDLK_a))
        self->position = glms_vec3_sub(
                            self->position, 
                            glms_vec3_scale(
                                glms_vec3_cross(
                                    self->front, 
                                    self->up
                                ),
                                GL_CAMERA_SPEED * dt
                            ) 
                        );
    if (window_keyboard_is_key_pressed(win, SDLK_d))
        self->position = glms_vec3_add(
                            self->position, 
                            glms_vec3_scale(
                                glms_vec3_cross(
                                    self->front, 
                                    self->up
                                ),
                                GL_CAMERA_SPEED * dt
                            ) 
                        );

    const vec2f_t offset = glms_vec2_scale(
                                glms_vec2_sub(newmp, oldmp), 
                                GL_CAMERA_SENSITIVITY); 
    self->angle.yaw     += offset.x;
    self->angle.pitch   += offset.y;

    if (window_mouse_is_scroll_up(win))
        self->zoom -= GL_CAMERA_ZOOM_OFFSET;
    else if (window_mouse_is_scroll_down(win))
        self->zoom -= GL_CAMERA_ZOOM_OFFSET;

    vec3f_t front = {
        .x = cos(glm_rad(self->angle.yaw)) * cos(glm_rad(self->angle.pitch)),
        .y = sin(glm_rad(self->angle.pitch)),
        .z = sin(glm_rad(self->angle.yaw)) * cos(glm_rad(self->angle.pitch)),
    };

    self->front = glms_normalize(front);
    // normalize the vectors, because their length gets closer to 0 the more 
    // you look up or down which results in slower movement.
    self->right = glms_normalize(glms_cross(self->front, self->worldup));  
    self->up    = glms_normalize(glms_cross(self->right, self->front));

    /*printf(VEC3F_FMT"\n", VEC3F_ARG(self->position));*/
    oldmp = newmp;
}


matrix4f_t glcamera_getview(glcamera_t *self)
{
    switch(self->type)
    {
        case GLCAMERATYPE_PERSPECTIVE:
            self->__view = glms_lookat(
                            self->position, 
                            glms_vec3_add(
                                self->position, 
                                self->front
                            ), 
                            self->up);
        break;

        case GLCAMERATYPE_ORTHOGRAPHIC:
        default: eprint("type not accounted for");
    }

    return self->__view;
}

glcamera_t glcamera_perspective(const vec3f_t pos)
{
    glcamera_t o = {
        .type       = GLCAMERATYPE_PERSPECTIVE,
        .position   = pos,
        .front      = GL_CAMERA_DIRECTION_FRONT,
        .up         = glms_normalize(glms_cross(glms_normalize(glms_cross(GL_CAMERA_DIRECTION_FRONT, GL_CAMERA_DIRECTION_UP)), GL_CAMERA_DIRECTION_FRONT)),
        .right      = glms_normalize(glms_cross(GL_CAMERA_DIRECTION_FRONT, GL_CAMERA_DIRECTION_UP)),
        .worldup    = GL_CAMERA_DIRECTION_UP,
        .angle = {
            .yaw    = GL_CAMERA_YAW,
            .pitch  = GL_CAMERA_PITCH
        },
        .zoom       = GL_CAMERA_ZOOM,
        .__view       = MATRIX4F_IDENTITY,
    };
    return o;
}

glcamera_t glcamera_orthographic(const vec3f_t pos)
{
    matrix4f_t projection = {0};
    glcamera_t o = {
        .type       = GLCAMERATYPE_ORTHOGRAPHIC,
        .position   = pos,
        .front      = GL_CAMERA_DIRECTION_FRONT,
        .up         = glms_normalize(glms_cross(glms_normalize(glms_cross(GL_CAMERA_DIRECTION_FRONT, GL_CAMERA_DIRECTION_UP)), GL_CAMERA_DIRECTION_FRONT)),
        .right      = glms_normalize(glms_cross(GL_CAMERA_DIRECTION_FRONT, GL_CAMERA_DIRECTION_UP)),
        .worldup    = GL_CAMERA_DIRECTION_UP,
        .angle = {
            .yaw    = GL_CAMERA_YAW,
            .pitch  = GL_CAMERA_PITCH
        },
        .zoom       = GL_CAMERA_ZOOM,
        .__view       = MATRIX4F_IDENTITY,
    };
    return o;
}

#endif
