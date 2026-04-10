#version 330 core

layout (location = 0) in vec3 v_vtx; 
layout (location = 1) in vec3 v_normal; 
layout (location = 2) in vec2 v_uv; 
layout (location = 3) in vec3 i_translation;
layout (location = 4) in vec3 i_scale;
layout (location = 5) in vec4 i_color;
layout (location = 6) in vec4 i_uv;

uniform mat4 projection;
uniform mat4 view;

out vec4 color;
out vec2 uv;

void main()
{
    uv = i_uv.xy + (v_uv * i_uv.zw);
    color = i_color;
    vec3 transformed_pos = (v_vtx * i_scale) + i_translation;
    gl_Position = projection * view * vec4(transformed_pos, 1.0);
}
