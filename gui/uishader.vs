#version 330 core

layout (location = 0) in uvec2 vertices;
layout (location = 1) in vec4 color;

uniform mat4 projection;

out vec4 fragColor;

void main()
{
    fragColor = color;
    gl_Position = projection * vec4(vertices,-1.0, 1.0);
}
