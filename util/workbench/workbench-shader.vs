#version 330 core
layout (location = 0) in vec3 vertices;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform;

out vec4 WorldPos;

void main()
{
    WorldPos = projection * view * transform * vec4(vertices, 1.0);
    gl_Position = projection * view * transform * vec4(vertices, 1.0);
}

