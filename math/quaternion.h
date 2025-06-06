#include <math.h>
#include <poglib/basic.h> // For vec3f_t, matrix4f_t, etc.
#include "./la.h"

typedef versors quaternionf_t;

#define QUATERNIONF_IDENTITY (quaternionf_t){ .w = 1.0f, .x = 0.0f, .y = 0.0f, .z = 0.0f }

// Normalize a quaternion
quaternionf_t quaternionf_normalize(quaternionf_t q) {
    f32 mag = sqrtf(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if (mag > 0.0f) {
        q.w /= mag;
        q.x /= mag;
        q.y /= mag;
        q.z /= mag;
    }
    return q;
}

// Spherical linear interpolation (SLERP) between two quaternions
quaternionf_t quaternionf_slerp(quaternionf_t q1, quaternionf_t q2, f32 t) {
    // Ensure quaternions are normalized
    q1 = quaternionf_normalize(q1);
    q2 = quaternionf_normalize(q2);

    // Compute dot product
    f32 dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;

    // If dot is negative, use the opposite quaternion to avoid long path
    if (dot < 0.0f) {
        q2.w = -q2.w;
        q2.x = -q2.x;
        q2.y = -q2.y;
        q2.z = -q2.z;
        dot = -dot;
    }

    // If quaternions are very close, use linear interpolation to avoid numerical issues
    if (dot > 0.9995f) {
        quaternionf_t result = {
            .w = q1.w + t * (q2.w - q1.w),
            .x = q1.x + t * (q2.x - q1.x),
            .y = q1.y + t * (q2.y - q1.y),
            .z = q1.z + t * (q2.z - q1.z)
        };
        return quaternionf_normalize(result);
    }

    // Compute interpolation
    f32 theta = acosf(dot);
    f32 sin_theta = sinf(theta);
    f32 s1 = sinf((1.0f - t) * theta) / sin_theta;
    f32 s2 = sinf(t * theta) / sin_theta;

    quaternionf_t result = {
        .w = s1 * q1.w + s2 * q2.w,
        .x = s1 * q1.x + s2 * q2.x, .y = s1 * q1.y + s2 * q2.y,
        .z = s1 * q1.z + s2 * q2.z
    };
    return quaternionf_normalize(result);
}

// Convert quaternion to a 4x4 rotation matrix
matrix4f_t quaternionf_to_matrix4f(quaternionf_t q) {
    q = quaternionf_normalize(q);

    f32 xx = q.x * q.x;
    f32 xy = q.x * q.y;
    f32 xz = q.x * q.z;
    f32 xw = q.x * q.w;
    f32 yy = q.y * q.y;
    f32 yz = q.y * q.z;
    f32 yw = q.y * q.w;
    f32 zz = q.z * q.z;
    f32 zw = q.z * q.w;

    matrix4f_t m = MATRIX4F_IDENTITY;
    m.raw[0][0] = 1.0f - 2.0f * (yy + zz);
    m.raw[0][1] = 2.0f * (xy - zw);
    m.raw[0][2] = 2.0f * (xz + yw);
    m.raw[1][0] = 2.0f * (xy + zw);
    m.raw[1][1] = 1.0f - 2.0f * (xx + zz);
    m.raw[1][2] = 2.0f * (yz - xw);
    m.raw[2][0] = 2.0f * (xz - yw);
    m.raw[2][1] = 2.0f * (yz + xw);
    m.raw[2][2] = 1.0f - 2.0f * (xx + yy);
    return m;
}
