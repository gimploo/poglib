#version 330 core
out vec4 FragColor;
in vec4 WorldPos;

uniform vec3 cameraPos;
uniform vec4 color;

void main() {
    float gridFadeDist = 80.0; // Distance where grid starts disappearing
    float dist = length(WorldPos.xz - cameraPos.xz); // Distance on the floor plane
    float alpha = 1.0 - smoothstep(gridFadeDist * 0.5, gridFadeDist, dist);

    vec3 gridColor = color.rgb;

    // Optional: Highlight the X and Z center axes
    if(abs(WorldPos.x) < 0.01) gridColor = vec3(0.0, 0.0, 1.0); // Z-axis Blue
    if(abs(WorldPos.z) < 0.01) gridColor = vec3(1.0, 0.0, 0.0); // X-axis Red

    FragColor = vec4(gridColor, alpha);
}
