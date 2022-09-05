#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/application.h>

#define GL_CAMERA_DIRECTION_FORWARD    (vec3f_t ){0.0f, 0.0f, -1.0f}
#define GL_CAMERA_DIRECTION_UP         (vec3f_t ){0.0f, 1.0f, 0.0f }
#define GL_CAMERA_SPEED                0.5f

typedef enum glcamera_type {

    GLCAMERATYPE_ORTHOGRAPHIC,
    GLCAMERATYPE_PERSPECTIVE,
    GLCAMREATYPE_COUNT

} glcamera_type;

typedef struct glcamera_t {

    glcamera_type   type;
    vec3f_t         position;
    vec3f_t         direction;
    union {
        vec4f_t     quaternion;
        f32         angle;
    } rotation;
    matrix4f_t       view;
    struct {
        const char  uniform[32];
    } __cache;

} glcamera_t ;

glcamera_t      glcamera_perspective(const char *uniform_name, const vec3f_t pos);
glcamera_t      glcamera_orthographic(const char *uniform_name, const vec3f_t pos);
void            glcamera_process_input(glcamera_t *self, const f32 dt);
void            glcamera_update(glcamera_t *self, const glshader_t *);


/*-----------------------------------------------------------------------------
                       -- IMPLEMENTATION --
-----------------------------------------------------------------------------*/
#ifndef IGNORE_GL_CAMERA_IMPLEMENTATION
void glcamera_process_input(glcamera_t *self, const f32 dt)
{
    window_t *win = window_get_current_active_window();

    if (window_keyboard_is_key_pressed(win, 'w')) {
        self->position = vec3f_add(
                            self->position, 
                            vec3f_scale(
                                GL_CAMERA_DIRECTION_FORWARD, 
                                GL_CAMERA_SPEED * dt
                            )
                        );
    }
    if (window_keyboard_is_key_pressed(win, 's'))
        self->position = vec3f_sub(
                            self->position, 
                            vec3f_scale(
                                GL_CAMERA_DIRECTION_FORWARD, 
                                GL_CAMERA_SPEED *dt
                            )
                        );
    if (window_keyboard_is_key_pressed(win, 'a'))
        self->position = vec3f_sub(
                            self->position, 
                            vec3f_scale(
                                vec3f_normalize(
                                    vec3f_cross(
                                        GL_CAMERA_DIRECTION_FORWARD, 
                                        GL_CAMERA_DIRECTION_UP
                                    )
                                ),
                                GL_CAMERA_SPEED * dt
                            ) 
                        );
    if (window_keyboard_is_key_pressed(win, 'd'))
        self->position = vec3f_add(
                            self->position, 
                            vec3f_scale(
                                vec3f_normalize(
                                    vec3f_cross(
                                        GL_CAMERA_DIRECTION_FORWARD, 
                                        GL_CAMERA_DIRECTION_UP
                                    )
                                ),
                                GL_CAMERA_SPEED * dt
                            ) 
                        );
    printf(VEC3F_FMT"\n", VEC3F_ARG(&self->position));
}


void glcamera_update(glcamera_t *self, const glshader_t *shader)
{
    window_t *win = window_get_current_active_window();
    const vec3f_t camera_target = vec3f(window_mouse_get_norm_position(win));

    switch(self->type)
    {
        case GLCAMERATYPE_ORTHOGRAPHIC :{

            matrixf_t translation = matrixf_sum(
                                        matrix4f_identity(), 
                                        matrix4f_translation(self->position)
                                    );
            matrixf_t rotation = matrixf_multiply(
                                    matrix4f_identity(),
                                    matrix4f_rotation(
                                        self->rotation.angle, (vec3f_t ){0.0f, 0.0f, 1.0f})
                                 );
            matrixf_t transform = matrixf_multiply(rotation, translation);

            self->view = matrix4f_inverse(transform);
        } break;

        case GLCAMERATYPE_PERSPECTIVE:
            self->view   = matrix4f_lookat(
                                    self->position, 
                                    vec3f_add(
                                        self->position, 
                                        GL_CAMERA_DIRECTION_FORWARD
                                    ), 
                                    GL_CAMERA_DIRECTION_UP
                                );
        break;

        default: eprint("type not accounted for");
    }

    glshader_send_uniform_matrix4f(shader, self->__cache.uniform, self->view);
}

glcamera_t glcamera_perspective( const char *uniform_name, const vec3f_t pos)
{
    glcamera_t o = {
        .type       = GLCAMERATYPE_PERSPECTIVE,
        .position   = pos,
        .rotation   = vec4f(0.0f),
        .view       = matrix4f_identity(),
        .__cache = {
            .uniform    = {0},
        },
    };

    memcpy((char *)o.__cache.uniform, uniform_name, sizeof(o.__cache.uniform));
    return o;
}

glcamera_t glcamera_orthographic(const char *uniform_name, const vec3f_t pos)
{
    eprint("TODO this function apprantely doesnt compile for some reason");
    matrixf_t projection = {0};
    glcamera_t o = {
        .type       = GLCAMERATYPE_ORTHOGRAPHIC,
        .position   = pos,
        .rotation   = vec4f(0.0f),
        .view       = matrix4f_identity(),
        .__cache = {
            .uniform    = {0},
        },
    };

    memcpy((char *)o.__cache.uniform, uniform_name, sizeof(o.__cache.uniform));
    return o;
}

#endif
