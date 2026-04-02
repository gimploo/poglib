#version 330 core

out vec4 FragColor;
in vec4 v_Color;
in vec2 v_UV;
in float v_is_text;

uniform sampler2D fontTexture;

void main()
{
    if (v_is_text == 1.0) {
        // Sample the glyph mask (Red channel)
        FragColor = v_Color * texture(fontTexture, v_UV).r;
    } else {
        // Non-text elements (quads/buttons) still use their assigned color
        FragColor = v_Color;
    }
}
