#version 330 core
in vec4 color;
in vec2 tex_coord;

uniform sampler2D u_texture01;

out vec4 FragColor;

void main()
{
    FragColor = color;
}
