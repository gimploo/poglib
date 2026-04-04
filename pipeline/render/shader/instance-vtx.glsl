#version 330 core

layout (location = 0) in vec3 v_vtx; 
layout (location = 1) in vec3 v_normal; 
layout (location = 2) in vec3 i_translation;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * vec4(v_vtx + i_translation, 1.0);
}
