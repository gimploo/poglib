#version 430 core

layout (location = 0) in vec3 v_vtx; 
layout (location = 1) in vec3 v_normal; 
layout (location = 2) in vec2 v_uv; 

struct renderinstance_t {
    vec4 uv;          // box_t maps directly to a 4-component vector
    vec4 color;       // offset 16
    vec4 translation; // offset 32 (XYZ used for position, W is ignorable padding/ID)
    vec4 scale;       // offset 48 (XYZ used for dimensions)
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
    vec3 transformed_pos = (v_vtx * data.scale.xyz) + data.translation.xyz;
    gl_Position = projection * view * vec4(transformed_pos, 1.0);
}
