#version 330 core

out vec4 FragColor;

in vec4 fragColor;
in vec2 textCoord;

uniform sampler2D texture1;

void main()
{
    FragColor = vec4(
        fragColor.rgb * texture(texture1, textCoord).rgb, 
        texture(texture1, textCoord).a
    );
}

