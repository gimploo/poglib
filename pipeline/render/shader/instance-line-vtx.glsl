#version 430 core

layout (location = 0) in vec3 v_vtx; 

struct renderinstance_t {
    vec4 color;
    vec4 translation;
    vec4 rotation;
    vec4 scale;
};

layout(std430, binding = 0) buffer InstanceBuffer {
    renderinstance_t instances[];
};

uniform mat4 projection;
uniform mat4 view;

out vec4 color;

void main()
{
    renderinstance_t data = instances[gl_InstanceID];
    color = data.color;

    vec4 q = data.rotation;
    float tx = 2.0 * q.x, ty = 2.0 * q.y, tz = 2.0 * q.z;
    float twx = tx * q.w, twy = ty * q.w, twz = tz * q.w;
    float txx = tx * q.x, txy = ty * q.x, txz = tz * q.x;
    float tyy = ty * q.y, tyz = tz * q.y, tzz = tz * q.z;

    mat3 rot = mat3(
        1.0 - (tyy + tzz),      txy - twz,              txz + twy,
        txy + twz,              1.0 - (txx + tzz),      tyz - twx,
        txz - twy,              tyz + twx,              1.0 - (txx + tyy)
    );

    vec3 transformed_pos = (rot * (v_vtx * data.scale.xyz)) + data.translation.xyz;
    gl_Position = projection * view * vec4(transformed_pos, 1.0);
}
