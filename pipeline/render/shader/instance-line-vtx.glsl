#version 430 core

layout (location = 0) in vec3 v_pos; 
layout (location = 1) in vec4 v_color; 

uniform mat4 projection;
uniform mat4 view;

out vec4 color;

void main()
{
    color = v_color;
    gl_Position = projection * view * vec4(v_pos, 1.0);
}
