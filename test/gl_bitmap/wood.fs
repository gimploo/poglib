#version 430 core
in vec2 TexCoord;
in vec3 ourColor; 

out vec4 FragColor;

uniform sampler2D texture1;

void main()
{
    FragColor = vec4(ourColor, 1.0f);
}

