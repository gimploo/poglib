#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/application/gfx/gl/shader.h>

typedef enum glcamera_type {

    GLCAMERATYPE_ORTHOGRAPHIC,
    GLCAMERATYPE_PERSPECTIVE,
    GLCAMREATYPE_COUNT

} glcamera_type;

typedef struct glcamera_t {

    glcamera_type   type;
    vec3f_t         position;
    union {
        vec4f_t     quaternion; // quaternion
        f32         angle;
    } rotation;
    struct { 
        matrixf_t   view; 
        matrixf_t   projection; 
    } matrix;
    struct {
        glshader_t          * const shader;
        matrixf_t           viewprojection;
    } __cache;

} glcamera_t ;

glcamera_t      glcamera(glshader_t * const shader, glcamera_type type);
void            glcamera_update(glcamera_t *self);
matrixf_t       glcamera_get_viewprojection(glcamera_t *);



/*-----------------------------------------------------------------------------
                       -- IMPLEMENTATION --
-----------------------------------------------------------------------------*/
#ifndef IGNORE_GL_CAMERA_IMPLEMENTATION

matrixf_t glcamera_get_viewprojection(glcamera_t *self)
{
    return self->__cache.viewprojection;
}

void glcamera_update(glcamera_t *self)
{
    switch(self->type)
    {
        case GLCAMERATYPE_ORTHOGRAPHIC :{

            matrixf_t translation = matrixf_sum(
                                        MATRIX4F_IDENTITY, 
                                        matrix4f_translation(self->position)
                                    );
            matrixf_t rotation = matrixf_product(
                                    MATRIX4F_IDENTITY,
                                    matrix4f_rotation(
                                        self->rotation.angle, 'z')
                                 );
            matrixf_t transform = matrixf_product(rotation, translation);

            self->matrix.view               = matrix4f_inverse(transform);
            self->__cache.viewprojection    = matrixf_product(
                                                self->matrix.projection, 
                                                self->matrix.view);
        } break;

        case GLCAMERATYPE_PERSPECTIVE:
        break;

        default: eprint("type not accounted for");
    }

    glshader_set_uniform(
            self->__cache.shader, 
            "cameraViewProjection", 
            self->__cache.viewprojection, 
            matrixf_t );
}

glcamera_t glcamera(glshader_t * const shader, glcamera_type type)
{
    matrixf_t projection = {0};

    switch(type)
    {
        case GLCAMERATYPE_ORTHOGRAPHIC :{
            projection = matrix4f_orthographic(
                            -1.0f, 1.0f, 
                            -1.0f, 1.0f, 
                            -1.0f, 1.0f); 
        } break;

        case GLCAMERATYPE_PERSPECTIVE:
            eprint("TODO"); 
        break;

        default: eprint("type not accounted for");
    }


    return (glcamera_t ) {
        .type       = type,
        .position   = vec3f(0.0f),
        .rotation   = vec4f(0.0f),
        .matrix = {
            .view       = MATRIX4F_IDENTITY,
            .projection = projection
        },
        .__cache = {
            .shader         = shader,
            .viewprojection = matrixf_product(
                                projection, 
                                MATRIX4F_IDENTITY)
        },
    };
}

#endif
