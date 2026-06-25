#version 330 core

out vec4 FragColor;
in vec4 v_Color;
in vec2 v_UV;
in float v_is_text;
in vec2 v_quad_coord;
in float v_corner_radius;

uniform sampler2D fontTexture;

float rounded_box_sdf(vec2 p, vec2 size, float radius)
{
    vec2 d = abs(p) - size + radius;
    return length(max(d, 0.0)) - radius;
}

void main()
{
    if (v_is_text == 1.0) {
        FragColor = v_Color * texture(fontTexture, v_UV).r;
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
