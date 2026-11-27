#version 460 core

out vec4 FragColor;
in vec4 v_Color;
in vec2 v_UV;
in float v_is_text;
in vec2 v_quad_coord;
in float v_corner_radius;
in float v_has_texture;

layout(binding = 0) uniform sampler2D fontTexture;
layout(binding = 1) uniform sampler2D texture1;

float rounded_box_sdf(vec2 p, vec2 size, float radius)
{
    vec2 d = abs(p) - size + radius;
    return length(max(d, 0.0)) - radius;
}

void main()
{
    if (v_is_text > 0.5) {
        FragColor = v_Color * texture(fontTexture, v_UV).r;
    } else if (v_has_texture > 0.5) {
        FragColor = vec4(texture(texture1, vec2(v_UV.x, 1 - v_UV.y)).rrr, 1.0);
    } else {
        FragColor = v_Color;
        if (v_corner_radius > 0.0) {
            vec2 p = abs(v_quad_coord - 0.5);
            float d = rounded_box_sdf(p, vec2(0.5), v_corner_radius);
            float alpha = clamp(1.0 - d, 0.0, 1.0);
            FragColor.a *= alpha;
        }
    }
}
