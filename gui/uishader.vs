#version 330 core

// 1. Static Buffer: The 1x1 unit quad vertices (Divisor = 0)
layout (location = 0) in vec2 a_UnitPos; 

// 2. Instance Buffer: Per-element data (Divisor = 1)
layout (location = 1) in vec4 i_Dest;   // [x, y, w, h] in pixels
layout (location = 2) in vec4 i_Color;  // [r, g, b, a]

uniform mat4 projection;

out vec4 v_Color;

void main()
{
    v_Color = i_Color;

    // Instance UV calculation: Start + (UnitPos * Dimensions)

    // Instance Position calculation: 
    // We scale the unit quad by width/height and offset by x/y
    vec2 pixel_pos = i_Dest.xy + (a_UnitPos * i_Dest.zw);

    // We set Z to 0.0 or a specific layer if you use a depth buffer
    gl_Position = projection * vec4(pixel_pos, 0.0, 1.0);
}
