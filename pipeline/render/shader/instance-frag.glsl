#version 330 core

out vec4 FragColor;
in vec4 color;
in vec2 uv;

uniform sampler2D prototypeTexture;

void main()
{
    vec4 texColor = texture(prototypeTexture, uv);
    FragColor = color * texColor;
}

