#version 330 core

out vec4 FragColor;

in vec4 color;
in vec2 uv;
in vec3 FragPos;
in vec4 lightspace_position;

uniform sampler2D prototypeTexture;
uniform vec3 camerapos;
uniform bool has_texture;
uniform sampler2D shadowmap;

const float FADE_START = 40.0;
const float FADE_END   = 120.0;

void main()
{
    vec3 p = lightspace_position.xyz/lightspace_position.w;
    vec3 new_color = color.rgb * (texture(shadowmap, p.xy).r < p.z ? 0 : 1);

    if (has_texture) {
        FragColor = texture(prototypeTexture, uv) * vec4(new_color, color.a);
    } else {
        FragColor = vec4(new_color, color.a);
    }
}
