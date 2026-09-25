#version 330 core

layout (location = 0) in vec3 vertices;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform;

out vec3 Normal;
out vec3 FragPos;

void main()
{
    vec4 fragPos = transform * vec4(vertices, 1.0);

    FragPos = vec3(fragPos);
    Normal = normal;

    gl_Position = projection * view * fragPos;
}
