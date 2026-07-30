#version 330 core

out vec4 FragColor;

in vec4 color;
in vec2 uv;
in vec3 FragPos;

uniform sampler2D prototypeTexture;
uniform vec3 camerapos;

const float FADE_START = 40.0;
const float FADE_END   = 120.0;

void main()
{
    vec4 texColor = texture(prototypeTexture, uv);
/*

    // 2. Calculate distance from camera to fragment
    float dist = length(FragPos - camerapos);

    // 3. Compute fade factor (0.0 = close up, 1.0 = far away)
    float fade = smoothstep(FADE_START, FADE_END, dist);

    // 4. Define solid background floor color (matching average tile tone)
    vec4 baseFloorColor = vec4(0, 0, 0, 1.0);

    // 5. Blend texture into solid color at a distance
    vec4 blendedColor = mix(texColor, baseFloorColor, fade);
    FragColor = color * blendedColor;
*/

    FragColor = color * texColor;
}
