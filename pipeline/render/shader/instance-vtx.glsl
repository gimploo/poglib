#version 430 core

layout (location = 0) in vec3 v_vtx; 
layout (location = 1) in vec3 v_normal; 
layout (location = 2) in vec2 v_uv; 

struct renderinstance_t {
    vec4 uv;
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
out vec2 uv;

void main()
{
    renderinstance_t data = instances[gl_InstanceID];

    uv = data.uv.xy + (v_uv * data.uv.zw);
    color = data.color;

    float cx = cos(data.rotation.x), sx = sin(data.rotation.x);
    float cy = cos(data.rotation.y), sy = sin(data.rotation.y);
    float cz = cos(data.rotation.z), sz = sin(data.rotation.z);

    mat3 rot = mat3(
        cy*cz,  cz*sx*sy - cx*sz,  cx*cz*sy + sx*sz,
        cy*sz,  cx*cz + sx*sy*sz, -cz*sx + cx*sy*sz,
        -sy,     cy*sx,             cy*cx
    );

    vec3 transformed_pos = (rot * (v_vtx * data.scale.xyz)) + data.translation.xyz;
    gl_Position = projection * view * vec4(transformed_pos, 1.0);
}
