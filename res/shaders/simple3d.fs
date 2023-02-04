#version 330 core
out vec4 FragColor;

in vec2 TexCoord;


void main()
{
    vec3 objectColor = vec3(1.0, 0.5, 0.31);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
	// linearly interpolate between both textures (80% container, 20% awesomeface)
    FragColor = vec4(lightColor * objectColor, 1.0 );
}
