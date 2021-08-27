#version 330 core
in vec3 color;
in vec2 tex_coord;

uniform sampler2D u_texture01;

out vec4 FragColor;

void main()
{
    FragColor = vec4(color, 1.0f);
}
