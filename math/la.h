#pragma once
#include "shapes.h"
#include "vec.h"

//NOTE: Find the operation / transformation u need from here
//      -- https://github.com/recp/cglm/tree/master/include/cglm 

typedef struct matrixf_t matrixf_t ;


#define         matrixf(ARR, NROW, NCOL)       __impl_matrixf_init(&(ARR), sizeof(ARR), (NROW), (NCOL))
matrixf_t       matrix2f(vec2f_t vertex[2]);
matrixf_t       matrix3f(vec3f_t vertex[3]);
matrixf_t       matrix4f(vec4f_t vertex[4]);

matrixf_t       matrix4f_rotation(const f32 angle_in_radians, const char along_axis);
matrixf_t       matrix4f_translation(vec3f_t vec);
matrixf_t       matrix4f_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
matrixf_t       matrix4f_inverse(matrixf_t mat);

matrixf_t       matrixf_scale(matrixf_t matrix, f32 s);
matrixf_t       matrixf_sum(matrixf_t a, matrixf_t b);
matrixf_t       matrixf_product(matrixf_t a, matrixf_t b);
matrixf_t       matrixf_transpose(matrixf_t a);

#define         matrixf_add_row(PMATRIX, ELEM) __impl_matrixf_add_row((PMATRIX), &(ELEM), sizeof(ELEM))

void            matrixf_print(matrixf_t *m);

#define MATRIX4F_IDENTITY \
    matrix4f(\
        (vec4f_t [4]) {\
            1.0f,1.0f,1.0f,1.0f,\
            1.0f,1.0f,1.0f,1.0f,\
            1.0f,1.0f,1.0f,1.0f,\
            1.0f,1.0f,1.0f,1.0f,\
        })

#define MATRIX3F_IDENTITY \
    matrix3f(\
        (vec3f_t [3]) {\
            1.0f,1.0f,1.0f,\
            1.0f,1.0f,1.0f,\
            1.0f,1.0f,1.0f,\
        })

#define MATRIX2F_IDENTITY \
    matrix2f(\
        (vec2f_t [2]) {\
            1.0f,1.0f,\
            1.0f,1.0f,\
        })


#ifndef IGNORE_LA_IMPLEMENTATION

#define MAX_MATRIX_BUFFER_SIZE WORD / 2

struct matrixf_t {

    u32 nrow;
    u32 ncol;
    u8  buffer[MAX_MATRIX_BUFFER_SIZE];
};


void __impl_matrixf_add_row(matrixf_t *m, void *value, u64 value_size)
{
    assert(value);
    assert((m->nrow * m->ncol) * sizeof(f32) != MAX_MATRIX_BUFFER_SIZE);

    f32 *pos = (f32 *)m->buffer + (m->ncol * m->nrow);
    memcpy(pos, value, value_size);
    m->nrow++;
}

matrixf_t __impl_matrixf_init(void *array, u64 array_size, u32 nrow, u32 ncol)
{
    assert(ncol > 0);
    assert(nrow > 0);
    if(array_size > MAX_MATRIX_BUFFER_SIZE) eprint("array exceeds MAX_MATRIX_BUFFER_SIZE");
    if (array_size == 8 && ((ncol + nrow) != 3)) eprint("passed a pointer instead of the array ");

    matrixf_t o = {
        .nrow = nrow,
        .ncol = ncol,
    };

    memcpy(o.buffer, array, array_size);

    return o;
}

matrixf_t matrix2f(vec2f_t vertex[2])
{
    matrixf_t o = (matrixf_t ){
        .nrow = 2,
        .ncol = 2
    };
    memcpy(o.buffer, vertex, sizeof(vec2f_t ) * 2);


    return o;
}

matrixf_t matrix3f(vec3f_t vertex[3])
{
    matrixf_t o = (matrixf_t ){
        .nrow = 3,
        .ncol = 3
    };
    memcpy(o.buffer, vertex, sizeof(vec3f_t ) * 3);

    return o;

}

matrixf_t matrix4f(vec4f_t vertex[4])
{    
    matrixf_t o = (matrixf_t ){
        .nrow = 4,
        .ncol = 4
    };
    memcpy(o.buffer, vertex, sizeof(vec4f_t ) * 4);

    return o;
}

matrixf_t matrixf_product(matrixf_t a, matrixf_t b)
{
    if (a.ncol != b.nrow) eprint("num of columns in first matrix dosent match with num of rows in the second matrix");

    matrixf_t mul = {0};
    
    f32 *mata       = (f32 *)a.buffer;
    f32 *matb       = (f32 *)b.buffer;
    f32 *product    = (f32 *)mul.buffer;

    for (u32 i = 0; i < a.nrow; i++) {
        for (u32 j = 0; j < b.ncol; j++) {
            f32 sum = 0;
            for (u32 k = 0; k < a.ncol; k++)
                sum = sum + mata[i * a.ncol + k] * matb[k * b.ncol + j];
            product[i * b.ncol + j] = sum;
        }
    }

    mul.nrow = a.nrow;
    mul.ncol = b.ncol;
    return mul;
    
}

matrixf_t matrixf_sum(matrixf_t a, matrixf_t b)
{
    assert(a.ncol == b.ncol);
    assert(b.nrow == a.nrow);

    matrixf_t o = {
        .nrow = a.nrow,
        .ncol = a.ncol
    };

    f32 *mata = (f32 *)a.buffer;
    f32 *matb = (f32 *)b.buffer;
    f32 *arr = (f32 *)o.buffer;

    for (u32 i = 0; i < (a.nrow * a.ncol); i++)
    {
        arr[i] = mata[i] + matb[i];
    }

    return o;
}

matrixf_t matrix4f_rotation(const f32 angle_in_radians, const char along_axis)
{
    vec4f_t rot[4] = {0.0f};

    switch(tolower(along_axis))
    {
        case 'x':
            rot[0] = (vec4f_t ){ 1.0f, 0.0f, 0.0f, 0.0f };
            rot[1] = (vec4f_t ){ 0.0f, (f32)cos(angle_in_radians), (f32)-sin(angle_in_radians), 0.0f};
            rot[2] = (vec4f_t ){ 0.0f, (f32)sin(angle_in_radians), (f32)cos(angle_in_radians), 0.0f};
            rot[3] = (vec4f_t ){ 0.0f, 0.0f, 0.0f, 1.0f };
        break;
        case 'y':
            rot[0] = (vec4f_t ){ (f32)cos(angle_in_radians), 0.0f, (f32)sin(angle_in_radians), 0.0f};
            rot[2] = (vec4f_t ){ 0.0f, 1.0f, 0.0f, 0.0f };
            rot[1] = (vec4f_t ){ (f32)-sin(angle_in_radians), 0.0f, (f32)cos(angle_in_radians), 0.0f};
            rot[3] = (vec4f_t ){ 0.0f, 0.0f, 0.0f, 1.0f };
        break;
        case 'z':
            rot[0] = (vec4f_t ){ (f32) cos(angle_in_radians), (f32)-sin(angle_in_radians), 0.0f, 0.0f};
            rot[1] = (vec4f_t ){ (f32) sin(angle_in_radians), (f32)cos(angle_in_radians), 0.0f, 0.0f};
            rot[2] = (vec4f_t ){ 0.0f, 0.0f, 1.0f, 0.0f };
            rot[3] = (vec4f_t ){ 0.0f, 0.0f, 0.0f, 1.0f };
        break;

        default: eprint("along axis %c ?", tolower(along_axis));
    }


    matrixf_t o = {
        .nrow = 4,
        .ncol = 4
    };

    memcpy(o.buffer, rot, sizeof(rot));

    return o;
}

matrixf_t matrix4f_translation(vec3f_t vec)
{
    vec4f_t tran[4] = {
        [0] = { 1.0f, 0.0f, 0.0f, vec.cmp[X]},
        [1] = { 0.0f, 1.0f, 0.0f, vec.cmp[Y]},
        [2] = { 0.0f, 0.0f, 1.0f, vec.cmp[Z]},
        [3] = { 0.0f, 0.0f, 0.0f, 1.0f }
    };

    matrixf_t o = {
        .nrow = 4,
        .ncol = 4
    };

    memcpy(o.buffer, tran, sizeof(tran));

    return o;
}

void matrixf_print(matrixf_t *m)
{
    assert(m);

    u32 row     = m->nrow;
    u32 column  = m->ncol;

    f32 *result = (f32 *)m->buffer;

    for (u32 i = 0; i < (row * column); ++i) 
    {
         if (i % column == 0) printf("\n");
         printf("%f ", result[i]);
   }
    printf("\n");
}

matrixf_t matrixf_transpose(matrixf_t a)
{
    matrixf_t output = {0};

    f32 *ibuf = (f32 *)a.buffer;
    f32 *obuf = (f32 *)output.buffer;

    for (u32 i = 0; i < a.nrow; ++i )
    {
       for (u32 j = 0; j < a.ncol; ++j )
       {
          // Index in the original matrix.
          u32 index1 = i * a.ncol + j;

          // Index in the transpose matrix.
          u32 index2 = j * a.nrow + i;


          obuf[index2] = ibuf[index1];
       }
    }
    output.ncol = a.nrow;
    output.nrow = a.ncol;

    return output;
}

matrixf_t matrix4f_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    matrixf_t output = MATRIX4F_IDENTITY;

    f32 **buffer = (f32 **)output.buffer;

    buffer[0][0] = 2 / (right - left);
    buffer[1][1] = 2 / (top - bottom);
    buffer[2][2] = 1 / (far - near);
    buffer[3][0] = - (right + left) / (top - bottom);
    buffer[3][1] = - (top + bottom) / (top - bottom);
    buffer[3][2] = - near / (far - near);

    return output;
}

matrixf_t matrix4f_inverse(matrixf_t matrix)
{
    assert(matrix.nrow == matrix.ncol);
    assert(matrix.nrow == 4);

    matrixf_t output = {
        .nrow = 4,
        .ncol = 4,
        .buffer = {0}
    };

    f32 **dest = (f32 **)output.buffer;
    f32 **mat = (f32 **)matrix.buffer;

    float t[6];
    float det;
    float a = mat[0][0], b = mat[0][1], c = mat[0][2], d = mat[0][3],
    e = mat[1][0], f = mat[1][1], g = mat[1][2], h = mat[1][3],
    i = mat[2][0], j = mat[2][1], k = mat[2][2], l = mat[2][3],
    m = mat[3][0], n = mat[3][1], o = mat[3][2], p = mat[3][3];

    t[0] = k * p - o * l; t[1] = j * p - n * l; t[2] = j * o - n * k;
    t[3] = i * p - m * l; t[4] = i * o - m * k; t[5] = i * n - m * j;

    dest[0][0] =  f * t[0] - g * t[1] + h * t[2];
    dest[1][0] =-(e * t[0] - g * t[3] + h * t[4]);
    dest[2][0] =  e * t[1] - f * t[3] + h * t[5];
    dest[3][0] =-(e * t[2] - f * t[4] + g * t[5]);

    dest[0][1] =-(b * t[0] - c * t[1] + d * t[2]);
    dest[1][1] =  a * t[0] - c * t[3] + d * t[4];
    dest[2][1] =-(a * t[1] - b * t[3] + d * t[5]);
    dest[3][1] =  a * t[2] - b * t[4] + c * t[5];

    t[0] = g * p - o * h; t[1] = f * p - n * h; t[2] = f * o - n * g;
    t[3] = e * p - m * h; t[4] = e * o - m * g; t[5] = e * n - m * f;

    dest[0][2] =  b * t[0] - c * t[1] + d * t[2];
    dest[1][2] =-(a * t[0] - c * t[3] + d * t[4]);
    dest[2][2] =  a * t[1] - b * t[3] + d * t[5];
    dest[3][2] =-(a * t[2] - b * t[4] + c * t[5]);

    t[0] = g * l - k * h; t[1] = f * l - j * h; t[2] = f * k - j * g;
    t[3] = e * l - i * h; t[4] = e * k - i * g; t[5] = e * j - i * f;

    dest[0][3] =-(b * t[0] - c * t[1] + d * t[2]);
    dest[1][3] =  a * t[0] - c * t[3] + d * t[4];
    dest[2][3] =-(a * t[1] - b * t[3] + d * t[5]);
    dest[3][3] =  a * t[2] - b * t[4] + c * t[5];

    det = 1.0f / (a * dest[0][0] + b * dest[1][0]
        + c * dest[2][0] + d * dest[3][0]);

    return matrixf_scale(output, det);
}

matrixf_t matrixf_scale(matrixf_t matrix, f32 s)
{
    matrixf_t output = matrix;
    f32 *m = (f32 *) matrix.buffer;
    f32 *o = (f32 *) output.buffer;

    for (u32 i = 0; i < (matrix.nrow * matrix.ncol); i++)
        o[i] = m[i] * s;

    return output;
}

#endif

