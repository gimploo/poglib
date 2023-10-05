#pragma once
#include <poglib/basic.h>
#define CGLM_USE_ANONYMOUS_STRUCT 1
#include <poglib/external/cglm/struct.h>

//NOTE: Find the operation / transformation u need from here
//      -- https://github.com/recp/cglm/tree/master/include/cglm 

#define X 0
#define Y 1
#define Z 2
#define W 3

typedef ivec2s vec2i_t ;
typedef ivec3s vec3i_t ;
typedef ivec4s vec4i_t ;
typedef vec2s vec2f_t ;
typedef vec3s vec3f_t ;
typedef vec4s vec4f_t ;
typedef mat2s matrix2f_t ;
typedef mat3s matrix3f_t ;
typedef mat4s matrix4f_t ;

#define vec2f(x) (vec2f_t ){x,x}
#define vec3f(x) (vec3f_t ){x,x,x}
#define vec4f(x) (vec4f_t ){x,x,x,x}

//NOTE: these are lower type cast
#define vec2f_cast(X) ((vec2f_t ) { (X).x, (X).y })
#define vec3f_cast(X) ((vec3f_t ) { (X).x, (X).y, (X).z })
#define vec4f_cast(X) ((vec4f_t ) { (X).x, (X).y, (X).z, (X).w })

matrix4f_t      matrix4f_rotate(const matrix4f_t mat, const f32 angle_in_radians, const vec3f_t along_axis);
matrix4f_t      matrix4f_translate(const matrix4f_t mat, const vec3f_t vec);
matrix4f_t      matrix4f_perpective(const f32 fov, const f32 aspect, const f32 nearz, const f32 farz);
matrix4f_t      matrix4f_inverse(const matrix4f_t mat);
matrix4f_t      matrix4f_scale(const matrix4f_t matrix, const f32 s);

matrix4f_t      matrix4f_translation(const vec3f_t vec);
matrix4f_t      matrix4f_rotation(const f32 angle_in_radians, const vec3f_t axis);
matrix4f_t      matrix4f_lookat(const vec3f_t eye, const vec3f_t center, const vec3f_t up); 

#define         MATRIX4F_IDENTITY       glms_mat4_identity()

matrix4f_t      matrix4f_multiply(const matrix4f_t a, const matrix4f_t b);
matrix4f_t      matrix4f_transpose(const matrix4f_t a);

void            matrix4f_print(const char *message, const matrix4f_t m);



#ifndef IGNORE_LA_IMPLEMENTATION


matrix4f_t matrix4f_multiply(const matrix4f_t a, const matrix4f_t b)
{
    return glms_mat4_mul(a, b);
}

matrix4f_t matrix4f_rot(const f32 angle_in_radians, const vec3f_t axis)
{
    return glms_rotate(glms_mat4_identity(), angle_in_radians, axis);
}

matrix4f_t matrix4f_rotate(const matrix4f_t mat,const f32 angle_in_radians, const vec3f_t axis)
{
    return glms_rotate(mat, angle_in_radians, axis);
}

matrix4f_t matrix4f_translate(const matrix4f_t mat, const vec3f_t vec)
{
    return glms_translate(mat, vec);
}

void matrix4f_print(const char *message, const matrix4f_t m)
{
    printf("%s", message);
    glms_mat4_print(m, stdout);
}

matrix4f_t matrix4f_transpose(const matrix4f_t a)
{
    return glms_mat4_transpose(a);
}


matrix4f_t matrix4f_inverse(const matrix4f_t matrix)
{
    return glms_mat4_inv_fast(matrix);
}

matrix4f_t matrix4f_scale(const matrix4f_t matrix, const f32 s)
{
    return glms_mat4_scale(matrix, s);
}

matrix4f_t matrix4f_perpective(const f32 fovy, const f32 aspect, const f32 nearZ, const f32 farZ)
{
    return glms_perspective(fovy, aspect, nearZ, farZ);
}


matrix4f_t matrix4f_lookat(const vec3f_t eye, const vec3f_t center, const vec3f_t up) 
{
    return glms_lookat(eye, center, up);
}

matrix4f_t matrix4f_translation(const vec3f_t vec)
{
    return (matrix4f_t ){
         1.0f, 0.0f, 0.0f, vec.x,
         0.0f, 1.0f, 0.0f, vec.y,
         0.0f, 0.0f, 1.0f, vec.z,
         0.0f, 0.0f, 0.0f, 1.0f 
    };
}

#endif

