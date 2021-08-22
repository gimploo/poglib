#version 330 core
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec2 v_tex_coord;

out vec3 color;
out vec2 tex_coord;

void main()
{
    gl_Position = vec4(v_pos, 1.0f);
    color = v_color;
    tex_coord = v_tex_coord;
}
