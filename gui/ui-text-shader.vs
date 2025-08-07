#version 330 core

layout (location = 0) in vec3 vertices;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;

uniform mat4 projection;

out vec4 fragColor;
out vec2 textCoord;

void main()
{
    fragColor = color;
    textCoord = uv;
    gl_Position = projection * vec4(vertices, 1.0);
}
