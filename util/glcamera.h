#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#define WINDOW_SDL
#include <poglib/application.h>

#define GL_CAMERA_DIRECTION_FRONT    (vec3f_t ){0.0f, 0.0f, -1.0f}
#define GL_CAMERA_DIRECTION_UP       (vec3f_t ){0.0f, 1.0f, 0.0f }

const f32 GL_CAMERA_SPEED           =  20.0f;
const f32 GL_CAMERA_SENSITIVITY     =  100.0f;

typedef struct glcamera_t {

    vec3f_t         position;
    struct {
        vec3f_t         front;
        vec3f_t         up;
        vec3f_t         right;
        vec3f_t         worldup;
    } direction;
    vec2f_t         theta;
    matrix4f_t      __view;

    struct {
        vec3f_t  position;
        vec2f_t  theta;
    } __reset;

} glcamera_t ;

glcamera_t      glcamera_perspective(const vec3f_t pos, const vec2f_t theta);
void            glcamera_process_input(glcamera_t *self, const f32 dt);
matrix4f_t      glcamera_getview(glcamera_t *self);


/*-----------------------------------------------------------------------------
                       -- IMPLEMENTATION --
-----------------------------------------------------------------------------*/
#ifndef IGNORE_GL_CAMERA_IMPLEMENTATION

void __glcamera_update_directions(glcamera_t *self)
{
    self->direction.front = glms_normalize(
                                (vec3f_t ) {
                                    .x = cos(self->theta.y) * cos(self->theta.x),
                                    .y = sin(self->theta.x),
                                    .z = sin(self->theta.y) * cos(self->theta.x),
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
}


void glcamera_process_input(glcamera_t *self, const f32 dt)
{
    window_t *win = window_get_current_active_window();

    const vec2f_t newmp     = window_mouse_get_norm_position(win);
    static vec2f_t oldmp    = {0.0f};

    if (window_keyboard_is_key_pressed(win, SDLK_UP)) 
        self->position = glms_vec3_add(
                            self->position, 
                            glms_vec3_scale(
                                self->direction.front, 
                                GL_CAMERA_SPEED * dt
                            )
                        );

    if (window_keyboard_is_key_pressed(win, SDLK_DOWN))
        self->position = glms_vec3_sub(
                            self->position, 
                            glms_vec3_scale(
                                self->direction.front, 
                                GL_CAMERA_SPEED *dt
                            )
                        );
    if (window_keyboard_is_key_pressed(win, SDLK_LEFT))
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
    if (window_keyboard_is_key_pressed(win, SDLK_RIGHT))
        self->position = glms_vec3_add(
                            self->position, 
                            glms_vec3_scale(
                                glms_vec3_crossn(
                                    self->direction.front, 
                                    self->direction.up
                                ),
                                GL_CAMERA_SPEED * dt
                            ) 
                        );
    if (window_keyboard_is_key_pressed(win, SDLK_r)) {
        self->position = self->__reset.position;
        self->theta = self->__reset.theta;
    }

    // Looking with the mouse by holding the left mouse button
    {
        const f32 angle = radians(
                glms_vec2_dot(newmp, oldmp) / 
                (glms_vec2_norm(oldmp) * glms_vec2_norm(newmp)));
        const vec2f_t direction = glms_vec2_normalize(glms_vec2_sub(newmp, oldmp));
                                    
        if (window_mouse_button_is_pressed(win, SDL_MOUSEBUTTON_LEFT)) {
            self->theta.y += wrap_angle(direction.x * angle * GL_CAMERA_SENSITIVITY * dt);
            self->theta.x += wrap_angle(direction.y * angle * GL_CAMERA_SENSITIVITY * dt);
        } 
    }

    __glcamera_update_directions(self);

    logging("Camera Pos: "VEC3F_FMT " | " "Angle: " VEC2F_FMT, 
            VEC3F_ARG(self->position), VEC2F_ARG(self->theta));

    oldmp = newmp;
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

glcamera_t glcamera_perspective(const vec3f_t pos, const vec2f_t theta)
{
    glcamera_t o = {
        .position   = pos,
        .direction = {
            .front      = GL_CAMERA_DIRECTION_FRONT,
            .up         = glms_normalize(glms_cross(glms_normalize(glms_cross(GL_CAMERA_DIRECTION_FRONT, GL_CAMERA_DIRECTION_UP)), GL_CAMERA_DIRECTION_FRONT)),
            .right      = glms_normalize(glms_cross(GL_CAMERA_DIRECTION_FRONT, GL_CAMERA_DIRECTION_UP)),
            .worldup    = GL_CAMERA_DIRECTION_UP,
        },
        .theta          = theta,
        .__view       = MATRIX4F_IDENTITY,
        .__reset = {
            .position = pos,
            .theta = theta
        }
    };

    logging("[CAMERA] left click look around and wasd to move the camera\n");
    return o;
}

#endif
