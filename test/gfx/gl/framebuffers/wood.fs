#version 330 core
in vec3 color;
in vec2 tex_coord;

uniform sampler2D texture1;

out vec4 FragColor;

void main()
{
    FragColor = texture(texture1, tex_coord);// + vec4(color, 1.0f);
}
