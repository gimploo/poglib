#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 rotation;
uniform mat4 translation;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    /*gl_Position = projection * translation * rotation * vec4(aPos, 1.0f);*/
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
