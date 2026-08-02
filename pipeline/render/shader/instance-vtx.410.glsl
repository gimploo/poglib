#version 410 core

// macOS downgrade of instance-vtx.glsl (GL 4.3 SSBO -> GL 4.1 instanced attributes).
// Renderinstance data arrives as per-instance vertex attributes at locations 3..7
// (set up by glinstancebuffer_bind with glVertexAttribDivisor).

layout (location = 0) in vec3 v_vtx; 
layout (location = 1) in vec3 v_normal; 
layout (location = 2) in vec2 v_uv; 

layout (location = 3) in vec4 i_uv;
layout (location = 4) in vec4 i_color;
layout (location = 5) in vec4 i_translation;
layout (location = 6) in vec4 i_rotation;
layout (location = 7) in vec4 i_scale;

uniform mat4 projection;
uniform mat4 view;

out vec4 color;
out vec2 uv;
out vec3 normal;
out vec3 FragPos;

void main()
{
    uv = i_uv.xy + (v_uv * i_uv.zw);
    color = i_color;

    vec4 q = i_rotation;
    float tx = 2.0 * q.x, ty = 2.0 * q.y, tz = 2.0 * q.z;
    float twx = tx * q.w, twy = ty * q.w, twz = tz * q.w;
    float txx = tx * q.x, txy = ty * q.x, txz = tz * q.x;
    float tyy = ty * q.y, tyz = tz * q.y, tzz = tz * q.z;

    mat3 rot = mat3(
        1.0 - (tyy + tzz),      txy + twz,              txz - twy,
        txy - twz,              1.0 - (txx + tzz),      tyz + twx,
        txz + twy,              tyz - twx,              1.0 - (txx + tyy)
    );

    vec3 transformed_pos = (rot * (v_vtx * i_scale.xyz)) + i_translation.xyz;
    FragPos = transformed_pos;
    normal = v_normal;
    gl_Position = projection * view * vec4(transformed_pos, 1.0);
}
