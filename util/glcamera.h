#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/application/gfx/gl/shader.h>

#define GL_CAMERA_DIRECTION_FORWARD    (vec3f_t ){0.0f, 0.0f, 1.0f}
#define GL_CAMERA_DIRECTION_UP         (vec3f_t ){0.0f, 1.0f, 0.0f }

typedef enum glcamera_type {

    GLCAMERATYPE_ORTHOGRAPHIC,
    GLCAMERATYPE_PERSPECTIVE,
    GLCAMREATYPE_COUNT

} glcamera_type;

typedef struct glcamera_t {

    glcamera_type   type;
    vec3f_t         position;
    union {
        vec4f_t     quaternion;
        f32         angle;
    } rotation;
    struct { 
        matrixf_t   view; 
        matrixf_t   projection; 
    } matrix;
    struct {
        matrixf_t   viewprojection;
        const char  uniform[32];
    } __cache;

} glcamera_t ;

glcamera_t      glcamera_perspective(const char *uniform_name, const vec3f_t pos, const f32 fov, const f32 aspect);
glcamera_t      glcamera_orthographic(const char *uniform_name, const vec3f_t pos);
void            glcamera_update(glcamera_t *self, const glshader_t *shader);
matrixf_t       glcamera_get_viewprojection(const glcamera_t *);



/*-----------------------------------------------------------------------------
                       -- IMPLEMENTATION --
-----------------------------------------------------------------------------*/
#ifndef IGNORE_GL_CAMERA_IMPLEMENTATION

matrixf_t glcamera_get_viewprojection(const glcamera_t *self)
{
    return self->__cache.viewprojection;
}

void glcamera_update(glcamera_t *self, const glshader_t *shader)
{
    switch(self->type)
    {
        case GLCAMERATYPE_ORTHOGRAPHIC :{

            matrixf_t translation = matrixf_sum(
                                        matrix4f_identity(), 
                                        matrix4f_translation(self->position)
                                    );
            matrixf_t rotation = matrixf_product(
                                    matrix4f_identity(),
                                    matrix4f_rotation(
                                        self->rotation.angle, 'z')
                                 );
            matrixf_t transform = matrixf_product(rotation, translation);

            self->matrix.view = matrix4f_inverse(transform);
        } break;

        case GLCAMERATYPE_PERSPECTIVE:
            self->matrix.view = matrix4f_lookat(
                                   self->position, 
                                   vec3f_add( self->position, 
                                       GL_CAMERA_DIRECTION_FORWARD
                                   ), 
                                   GL_CAMERA_DIRECTION_UP);
        break;

        default: eprint("type not accounted for");
    }

    self->__cache.viewprojection = matrixf_product(
                                   self->matrix.projection, 
                                   self->matrix.view);

    glshader_send_uniform_matrix4f(shader, self->__cache.uniform, self->__cache.viewprojection);
}

glcamera_t glcamera_perspective(const char *uniform_name, const vec3f_t pos, const f32 fov, const f32 aspect)
{
    const f32 zNear = 0.01f;
    const f32 zFar = 1000.0f;

    matrixf_t projection = matrix4f_perpective(
                            fov, aspect, zNear, zFar);

    glcamera_t o = {
        .type       = GLCAMERATYPE_PERSPECTIVE,
        .position   = pos,
        .rotation   = vec4f(0.0f),
        .matrix = {
            .view       = matrix4f_identity(),
            .projection = projection
        },
        .__cache = {
            .viewprojection = matrixf_product(
                                projection, 
                                matrix4f_identity()),
            .uniform        = {0},
        },
    };
    memcpy((char *)o.__cache.uniform, uniform_name, sizeof(o.__cache.uniform));
    return o;
}

glcamera_t glcamera_orthographic(const char *uniform_name, const vec3f_t pos)
{
    eprint("TODO this function apprantely doesnt compile for some reason");
    matrixf_t projection = {0};
    /*matrixf_t projection = matrix4f_orthographic(*/
                            /*-1.0f, 1.0f, */
                            /*-1.0f, 1.0f, */
                            /*-1.0f, 1.0f); */

    glcamera_t o = {
        .type       = GLCAMERATYPE_ORTHOGRAPHIC,
        .position   = pos,
        .rotation   = vec4f(0.0f),
        .matrix = {
            .view       = matrix4f_identity(),
            .projection = projection
        },
        .__cache = {
            .viewprojection = matrixf_product(
                                projection, 
                                matrix4f_identity()),
            .uniform        = {0},
        },
    };

    memcpy((char *)o.__cache.uniform, uniform_name, sizeof(o.__cache.uniform));
    return o;
}

#endif
