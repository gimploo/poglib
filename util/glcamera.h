#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#define WINDOW_SDL
#include <poglib/application.h>

#define GL_CAMERA_DIRECTION_FRONT    (vec3f_t ){0.0f, 0.0f, -1.0f}
#define GL_CAMERA_DIRECTION_UP       (vec3f_t ){0.0f, 1.0f, 0.0f }

const f32 GL_CAMERA_SPEED           =  20.0f;
const f32 GL_CAMERA_SENSITIVITY     =  1.f;

typedef struct glcamera_t {

    vec3f_t         position;
    vec2f_t         theta;
    struct {
        vec3f_t         front;
        vec3f_t         up;
        vec3f_t         right;
        vec3f_t         worldup;
    } direction;
    matrix4f_t      __view;

    struct {
        vec3f_t     position;
        vec2f_t     theta;
    } __reset;

} glcamera_t ;

glcamera_t      glcamera_perspective(const vec3f_t pos, const vec2f_t theta);
void            glcamera_process_input(glcamera_t *self, const f32 dt);
matrix4f_t      glcamera_getview(glcamera_t *self);


/*-----------------------------------------------------------------------------
                       -- IMPLEMENTATION --
-----------------------------------------------------------------------------*/
#ifndef IGNORE_GL_CAMERA_IMPLEMENTATION

void __glcamera_update_directions(glcamera_t *self, const vec2f_t rot)
{
    const vec4f_t front = glms_mat4_mulv(
            glms_mat4_mul(
                matrix4f_rot(rot.y, (vec3f_t){1.f, 0.f, 0.f}), //y
                matrix4f_rot(rot.x, (vec3f_t){0.f, 1.f, 0.f}) //x
            ), 
            (vec4f_t ) {
                self->direction.front.x,
                self->direction.front.y,
                self->direction.front.z,
                1.0f
            });

    self->direction.front = glms_normalize(
            (vec3f_t ){
                front.x,
                front.y,
                front.z
            });

    // normalize the vectors, because their length gets closer to 0 the more 
    // you look up or down which results in slower movement.
    self->direction.right = glms_normalize(
                                glms_cross(
                                    self->direction.front, 
                                    self->direction.worldup
                                ));  
    self->direction.up    = glms_normalize(
                                glms_cross(
                                    self->direction.right, 
                                    self->direction.front
                                ));

    self->theta = glms_vec2_add(self->theta, rot);
}


void glcamera_process_input(glcamera_t *self, const f32 dt)
{
    window_t *win = window_get_current_active_window();

    if (window_keyboard_is_key_pressed(win, SDLK_w)) 
        self->position = glms_vec3_add(
                            self->position, 
                            glms_vec3_scale(
                                self->direction.front, 
                                GL_CAMERA_SPEED * dt
                            )
                        );

    if (window_keyboard_is_key_pressed(win, SDLK_s))
        self->position = glms_vec3_sub(
                            self->position, 
                            glms_vec3_scale(
                                self->direction.front, 
                                GL_CAMERA_SPEED *dt
                            )
                        );
    if (window_keyboard_is_key_pressed(win, SDLK_a))
        self->position = glms_vec3_sub(
                            self->position, 
                            glms_vec3_scale(
                                glms_vec3_crossn(
                                    self->direction.front, 
                                    self->direction.up
                                ),
                                GL_CAMERA_SPEED * dt
                            ) 
                        );
    if (window_keyboard_is_key_pressed(win, SDLK_d))
        self->position = glms_vec3_add(
                            self->position, 
                            glms_vec3_scale(
                                glms_vec3_crossn(
                                    self->direction.front, 
                                    self->direction.up
                                ),
                                GL_CAMERA_SPEED * dt
                            ));

    if (window_keyboard_is_key_pressed(win, SDLK_r)) {

        self->position        = self->__reset.position;
        self->theta           = self->__reset.theta;
        self->direction.front = GL_CAMERA_DIRECTION_FRONT,
        self->direction.up    = glms_normalize(glms_cross(glms_normalize(glms_cross(GL_CAMERA_DIRECTION_FRONT, GL_CAMERA_DIRECTION_UP)), GL_CAMERA_DIRECTION_FRONT)),
        self->direction.right = glms_normalize(glms_cross(GL_CAMERA_DIRECTION_FRONT, GL_CAMERA_DIRECTION_UP)),

        __glcamera_update_directions(self, self->theta);
    }

    // Looking with the mouse by holding the left mouse button
    {
        if (window_mouse_button_is_pressed(win, SDL_MOUSEBUTTON_LEFT)) {
            const vec2f_t angle = glms_vec2_normalize((vec2f_t ){
                        radians(win->mouse.rel.x),
                        radians(win->mouse.rel.y),
                    });
            const vec2f_t theta = (vec2f_t ){
                .x = wrap_angle(angle.x * GL_CAMERA_SENSITIVITY * dt),
                .y = wrap_angle(angle.y * GL_CAMERA_SENSITIVITY * dt)
            };
            __glcamera_update_directions(self, theta);
        } 
    }

    logging("Camera Pos: "VEC3F_FMT " | " "Angle: " VEC2F_FMT, 
            VEC3F_ARG(self->position), VEC2F_ARG(self->theta));
}


matrix4f_t glcamera_getview(glcamera_t *self)
{
    self->__view = glms_lookat(
            self->position, 
            glms_vec3_add(
                self->position, 
                self->direction.front
                ), 
            self->direction.up);
    return self->__view;
}

glcamera_t glcamera_perspective(const vec3f_t pos, const vec2f_t radians)
{
    glcamera_t o = {
        .position   = pos,
        .theta      = radians,
        .direction = {
            .front      = GL_CAMERA_DIRECTION_FRONT,
            .up         = {0},
            .right      = {0},
            .worldup    = GL_CAMERA_DIRECTION_UP,
        },
        .__view         = MATRIX4F_IDENTITY,
        .__reset = {
            .position   = pos,
            .theta      = radians,
        }
    };

    __glcamera_update_directions(&o, radians);

    logging("[CAMERA] left click look around and wasd to move the camera\n");
    return o;
}

#endif
