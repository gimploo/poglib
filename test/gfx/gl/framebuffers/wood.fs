#version 330 core
in vec3 color;
in vec2 tex_coord;

uniform sampler2D u_texture01;

out vec4 FragColor;

void main()
{
    FragColor = texture(u_texture01, tex_coord);// + vec4(color, 1.0f);
}
