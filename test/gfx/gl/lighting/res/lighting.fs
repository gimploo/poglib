#version 330 core
out vec4 FragColor;
in vec4 lightColor;

void main()
{
    FragColor = lightColor; // set all 4 vector values to 1.0
}
