#pragma once
#include "shapes.h"
#include "vec.h"


typedef struct matrixf_t matrixf_t ;


#define         matrixf_init(ARR, NROW, NCOL)       __impl_matrixf_init(&(ARR), sizeof(ARR), (NROW), (NCOL))
matrixf_t       matrix2f_init(vec2f_t vertex[2]);
matrixf_t       matrix3f_init(vec3f_t vertex[3]);
matrixf_t       matrix4f_init(vec4f_t vertex[4]);
matrixf_t       matrix4f_rotation_init(f32 angle_in_radians);
matrixf_t       matrix4f_translation_init(vec3f_t vec);


matrixf_t       matrixf_sum(matrixf_t a, matrixf_t b);
matrixf_t       matrixf_product(matrixf_t a, matrixf_t b);
matrixf_t       matrixf_transpose(matrixf_t a);

#define         matrixf_add_row(PMATRIX, ELEM) __impl_matrixf_add_row((PMATRIX), &(ELEM), sizeof(ELEM))

void            matrixf_print(matrixf_t *m);


#ifndef IGNORE_LA_IMPLEMENTATION

#define MAX_MATRIX_BUFFER_SIZE KB

struct matrixf_t {

    u8 buffer[MAX_MATRIX_BUFFER_SIZE];
    u8 nrow;
    u8 ncol;
};

void __impl_matrixf_add_row(matrixf_t *m, void *value, u64 value_size)
{
    assert(value);
    assert((m->nrow * m->nrow) * sizeof(f32) != MAX_MATRIX_BUFFER_SIZE);

    f32 *pos = (f32 *)m->buffer + (m->ncol * m->nrow);
    memcpy(pos, value, value_size);
    m->nrow++;
}

matrixf_t __impl_matrixf_init(void *array, u64 array_size, u8 nrow, u8 ncol)
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

matrixf_t matrix2f_init(vec2f_t vertex[2])
{
    matrixf_t o = (matrixf_t ){
        .nrow = 2,
        .ncol = 2
    };
    memcpy(o.buffer, vertex, sizeof(vec2f_t ) * 2);


    return o;
}

matrixf_t matrix3f_init(vec3f_t vertex[3])
{
    matrixf_t o = (matrixf_t ){
        .nrow = 3,
        .ncol = 3
    };
    memcpy(o.buffer, vertex, sizeof(vec3f_t ) * 3);

    return o;

}

matrixf_t matrix4f_init(vec4f_t vertex[4])
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

matrixf_t matrix4f_rotation_init(f32 angle_in_radians)
{
    vec4f_t rot[4] = {
        [0] = { (f32) cos(angle_in_radians), (f32)-sin(angle_in_radians), 0.0f, 0.0f},
        [1] = { (f32) sin(angle_in_radians), (f32)cos(angle_in_radians), 0.0f, 0.0f},
        [2] = { 0.0f, 0.0f, 1.0f, 0.0f },
        [3] = { 0.0f, 0.0f, 0.0f, 1.0f }
    };

    matrixf_t o = {
        .nrow = 4,
        .ncol = 4
    };

    memcpy(o.buffer, rot, sizeof(rot));

    return o;
}

matrixf_t matrix4f_translation_init(vec3f_t vec)
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

#endif

