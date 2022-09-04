#pragma once
#include "shapes.h"
#include "vec.h"

//NOTE: Find the operation / transformation u need from here
//      -- https://github.com/recp/cglm/tree/master/include/cglm 

#define MAX_MATRIX_BUFFER_SIZE      WORD / 2

typedef struct matrixf_t {

    u8  nrow;
    u8  ncol;
    u8  buffer[MAX_MATRIX_BUFFER_SIZE];

} matrixf_t ;

#define         matrixf(ARR, NROW, NCOL)        __impl_matrixf_init(&(ARR), sizeof(ARR), (NROW), (NCOL))
#define         matrix2f(ARR)                   __impl_matrix2f(ARR, sizeof(ARR))
#define         matrix3f(ARR)                   __impl_matrix3f(ARR, sizeof(ARR))
#define         matrix4f(ARR)                   __impl_matrix4f((ARR), sizeof(ARR))

matrixf_t       matrix4f_rotate(const matrixf_t mat, const f32 angle_in_radians, const vec3f_t along_axis);
matrixf_t       matrix4f_translate(const matrixf_t mat, const vec3f_t vec);
matrixf_t       matrix4f_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 far, f32 near);
matrixf_t       matrix4f_perspective(const f32 fov, const f32 aspect, const f32 nearz, const f32 farz);
matrixf_t       matrix4f_inverse(const matrixf_t mat);
matrixf_t       matrix4f_scale(const matrixf_t matrix, const f32 s);

matrixf_t       matrix4f_translation(const vec3f_t vec);
matrixf_t       matrix4f_rotation(const f32 angle_in_radians, const vec3f_t axis);
matrixf_t       matrix2f_identity(void);
matrixf_t       matrix3f_identity(void);
matrixf_t       matrix4f_identity(void);

matrixf_t       matrix4f_lookat(const vec3f_t eye, const vec3f_t center, const vec3f_t up); 

matrixf_t       matrixf_sum(const matrixf_t a, const matrixf_t b);
matrixf_t       matrixf_multiply(const matrixf_t a, const matrixf_t b);
matrixf_t       matrixf_transpose(const matrixf_t a);

#define         matrixf_copy_to_array(MAT, OUTPUT) memcpy(OUTPUT, MAT.buffer, sizeof(f32) * MAT.ncol * MAT.nrow)
void            matrixf_print(const matrixf_t *m);




#ifndef IGNORE_LA_IMPLEMENTATION


matrixf_t __impl_matrixf_init(const void *array, const u64 array_size, const u8 nrow, const u8 ncol)
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

matrixf_t __impl_matrix2f(const void *vertex, const u64 size)
{
    assert(size == sizeof(vec2f_t [2]));

    matrixf_t o = (matrixf_t ){
        .nrow = 2,
        .ncol = 2
    };
    memcpy(o.buffer, vertex, sizeof(vec2f_t ) * 2);


    return o;
}

matrixf_t __impl_matrix3f(const void *vertex, const u64 size)
{
    assert(size == sizeof(vec3f_t [3]));

    matrixf_t o = (matrixf_t ){
        .nrow = 3,
        .ncol = 3
    };
    memcpy(o.buffer, vertex, sizeof(vec3f_t ) * 3);

    return o;

}

matrixf_t __impl_matrix4f(const void *vertex, const u64 size)
{    
    assert(size == sizeof(vec4f_t [4]));

    matrixf_t o = (matrixf_t ){
        .nrow = 4,
        .ncol = 4
    };
    memcpy(o.buffer, vertex, sizeof(vec4f_t ) * 4);

    return o;
}

matrixf_t matrixf_multiply(const matrixf_t a, const matrixf_t b)
{
    if (a.ncol != b.nrow) eprint("num of columns in first matrix dosent match with num of rows in the second matrix");

    
    f32 *mata       = (f32 *)a.buffer;
    f32 *matb       = (f32 *)b.buffer;
    f32 product[MAX_MATRIX_BUFFER_SIZE] = {0};

    for (u32 i = 0; i < a.nrow; i++) {
        for (u32 j = 0; j < b.ncol; j++) {
            f32 sum = 0;
            for (u32 k = 0; k < a.ncol; k++)
                sum = sum + mata[i * a.ncol + k] * matb[k * b.ncol + j];
            product[i * b.ncol + j] = sum;
        }
    }

    return matrixf(product, a.nrow, b.ncol);
}

//Note: reason these arguments are this way is cuz msvc doesnt compile otherwise
matrixf_t matrix4f_orthographic(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Far, f32 Near)
{
    f32 buffer[4][4] = {0}; 
    matrixf_copy_to_array(matrix4f_identity(), buffer);

    buffer[0][0] = 2 / (Right - Left);
    buffer[1][1] = 2 / (Top - Bottom);
    buffer[2][2] = 1 / (Far - Near);
    buffer[3][0] = - (Right + Left) / (Top - Bottom);
    buffer[3][1] = - (Top + Bottom) / (Top - Bottom);
    buffer[3][2] = - Near / (Far - Near);

    return matrixf(buffer, 4, 4);
}

matrixf_t matrixf_sum(const matrixf_t a, const matrixf_t b)
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

matrixf_t matrix4f_rotation(const f32 angle_in_radians, const vec3f_t axis)
{
    vec4f_t rot[4] = {0};

    const f32 angle       = angle_in_radians;
    const f32 c           = cosf(angle);
    const vec3f_t axisn   = vec3f_normalize(axis);
    const vec3f_t v       = vec3f_scale(axisn, 1.0f - c);
    const vec3f_t vs      = vec3f_scale(axisn, sinf(angle));

    rot[0] = vec4f(vec3f_scale(axisn, v.cmp[0]));
    rot[1] = vec4f(vec3f_scale(axisn, v.cmp[1]));
    rot[2] = vec4f(vec3f_scale(axisn, v.cmp[2]));

    rot[0].cmp[0] += c;           rot[1].cmp[0] -= vs.cmp[2];   rot[2].cmp[0] += vs.cmp[1];
    rot[0].cmp[1] += vs.cmp[2];   rot[1].cmp[1] += c;           rot[2].cmp[1] -= vs.cmp[0];
    rot[0].cmp[2] -= vs.cmp[1];   rot[1].cmp[2] += vs.cmp[0];   rot[2].cmp[2] += c;

    rot[0].cmp[3] = rot[1].cmp[3] 
        = rot[2].cmp[3] = rot[3].cmp[0] 
        = rot[3].cmp[1] = rot[3].cmp[2] = 0.0f;
    rot[3].cmp[3] = 1.0f;

    return matrix4f(rot);
}

matrixf_t matrix4f_rotate(
        const matrixf_t mat, 
        const f32 angle_in_radians, 
        const vec3f_t axis)
{
    assert(mat.ncol == mat.nrow);
    assert(mat.ncol == 4);

    matrixf_t rot = matrix4f_rotation(angle_in_radians, axis);

    //NOTE: glm_mul_rot(mat4 m1, mat4 m2, mat4 dest) 
        f32 dest[4][4] = {0};
        f32 m1[4][4]; matrixf_copy_to_array(mat, m1);
        f32 m2[4][4]; matrixf_copy_to_array(rot, m2);

        f32 a00 = m1[0][0], a01 = m1[0][1], a02 = m1[0][2], a03 = m1[0][3],
        a10 = m1[1][0], a11 = m1[1][1], a12 = m1[1][2], a13 = m1[1][3],
        a20 = m1[2][0], a21 = m1[2][1], a22 = m1[2][2], a23 = m1[2][3],
        a30 = m1[3][0], a31 = m1[3][1], a32 = m1[3][2], a33 = m1[3][3],

        b00 = m2[0][0], b01 = m2[0][1], b02 = m2[0][2],
        b10 = m2[1][0], b11 = m2[1][1], b12 = m2[1][2],
        b20 = m2[2][0], b21 = m2[2][1], b22 = m2[2][2];

        dest[0][0] = a00 * b00 + a10 * b01 + a20 * b02;
        dest[0][1] = a01 * b00 + a11 * b01 + a21 * b02;
        dest[0][2] = a02 * b00 + a12 * b01 + a22 * b02;
        dest[0][3] = a03 * b00 + a13 * b01 + a23 * b02;

        dest[1][0] = a00 * b10 + a10 * b11 + a20 * b12;
        dest[1][1] = a01 * b10 + a11 * b11 + a21 * b12;
        dest[1][2] = a02 * b10 + a12 * b11 + a22 * b12;
        dest[1][3] = a03 * b10 + a13 * b11 + a23 * b12;

        dest[2][0] = a00 * b20 + a10 * b21 + a20 * b22;
        dest[2][1] = a01 * b20 + a11 * b21 + a21 * b22;
        dest[2][2] = a02 * b20 + a12 * b21 + a22 * b22;
        dest[2][3] = a03 * b20 + a13 * b21 + a23 * b22;

        dest[3][0] = a30;
        dest[3][1] = a31;
        dest[3][2] = a32;
        dest[3][3] = a33;

    return matrix4f(dest);
}

matrixf_t matrix4f_translate(const matrixf_t matrix, const vec3f_t vec)
{
    assert(matrix.ncol == matrix.nrow);
    assert(matrix.ncol == 4);

    vec4f_t mat[4] = {0};
    matrixf_copy_to_array(matrix, mat);

    const f32 trans[4][4] = {
        1.0f, 0.0f, 0.0f, vec.cmp[X],
        0.0f, 1.0f, 0.0f, vec.cmp[Y],
        0.0f, 0.0f, 1.0f, vec.cmp[Z],
        0.0f, 0.0f, 0.0f, 1.0f
    };

    mat[0] = vec4f_add(mat[0], vec4f_scale(mat[3], vec.cmp[0]));
    mat[1] = vec4f_add(mat[1], vec4f_scale(mat[3], vec.cmp[1]));
    mat[2] = vec4f_add(mat[2], vec4f_scale(mat[3], vec.cmp[2]));

    return matrix4f(mat);
}

void matrixf_print(const matrixf_t *m)
{
    assert(m);

    const u8 row     = m->nrow;
    const u8 column  = m->ncol;

    f32 *result = (f32 *)m->buffer;

    for (u32 i = 0; i < (row * column); ++i) 
    {
         if (i % column == 0) printf("\n");
         printf("%f ", result[i]);
   }
    printf("\n");
}

matrixf_t matrixf_transpose(const matrixf_t a)
{
    f32 obuf[MAX_MATRIX_BUFFER_SIZE] = {0};

    f32 *ibuf = (f32 *)a.buffer;

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
    return matrixf(obuf, a.ncol, a.nrow);
}


matrixf_t matrix4f_inverse(const matrixf_t matrix)
{
    assert(matrix.nrow == matrix.ncol);
    assert(matrix.nrow == 4);

    f32 dest[4][4] = {0};
    f32 mat[4][4] = {0};
    matrixf_copy_to_array(matrix, mat);

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

    return matrix4f_scale(matrixf(dest, 4, 4), det);
}

matrixf_t matrix4f_scale(const matrixf_t matrix, const f32 s)
{
    const f32 scale[4][4] = {
        s, 0.0f, 0.0f, 0.0f,
        0.0f, s, 0.0f, 0.0f,
        0.0f, 0.0f, s, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return matrixf_multiply(matrix4f(scale), matrix);
}

matrixf_t matrix4f_perspective(const f32 fovy, const f32 aspect, const f32 nearZ, const f32 farZ)
{
    const f32 f  = 1.0f / tanf(fovy * 0.5f);
    const f32 fn = 1.0f / (nearZ - farZ);

    f32 dest[4][4] = {0};

    dest[0][0] = f / aspect;
    dest[1][1] = f;
    dest[2][2] = farZ * fn;
    dest[2][3] =-1.0f;
    dest[3][2] = nearZ * farZ * fn;

    return matrix4f(dest);
}

matrixf_t matrix4f_identity(void)
{
    const f32 mati[4][4] = {
        1.0f,0.0f,0.0f,0.0f, 
        0.0f,1.0f,0.0f,0.0f, 
        0.0f,0.0f,1.0f,0.0f, 
        0.0f,0.0f,0.0f,1.0f 
    };

    return matrix4f(mati);
}

matrixf_t matrix2f_identity(void)
{
    const f32 mati[2][2] = {
        1.0f,0.0f, 
        0.0f,1.0f, 
    };

    return matrix2f(mati);
}

matrixf_t matrix3f_identity(void)
{
    const f32 mati[3][3] = {
        1.0f,0.0f,0.0f, 
        0.0f,1.0f,0.0f, 
        0.0f,0.0f,1.0f, 
    };

    return matrix3f(mati);
}

matrixf_t matrix4f_lookat(const vec3f_t eye, const vec3f_t center, const vec3f_t up) 
{
    f32 dest[4][4] = {0};

    const vec3f_t f = vec3f_normalize(vec3f_sub(center, eye));
    const vec3f_t s = vec3f_normalize(vec3f_cross(f, up));
    const vec3f_t u = vec3f_cross(s, f);

    dest[0][0] = s.cmp[0];
    dest[0][1] = u.cmp[0];
    dest[0][2] =-f.cmp[0];
    dest[1][0] = s.cmp[1];
    dest[1][1] = u.cmp[1];
    dest[1][2] =-f.cmp[1];
    dest[2][0] = s.cmp[2];
    dest[2][1] = u.cmp[2];
    dest[2][2] =-f.cmp[2];
    dest[3][0] =-vec3f_dot(s, eye);
    dest[3][1] =-vec3f_dot(u, eye);
    dest[3][2] = vec3f_dot(f, eye);
    dest[0][3] = dest[1][3] = dest[2][3] = 0.0f;
    dest[3][3] = 1.0f;

    return matrix4f(dest);
}

matrixf_t matrix4f_translation(const vec3f_t vec)
{
    const f32 tran[4][4] = {
         1.0f, 0.0f, 0.0f, vec.cmp[X],
         0.0f, 1.0f, 0.0f, vec.cmp[Y],
         0.0f, 0.0f, 1.0f, vec.cmp[Z],
         0.0f, 0.0f, 0.0f, 1.0f 
    };

    return matrix4f(tran);
}

#endif

