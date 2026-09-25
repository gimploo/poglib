#version 420 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

struct pointlight_t {
    vec4 position; // xyz: position, w: pad/unused
    vec4 color;    // rgb: color,    w: intensity
    vec4 params;   // x: constant, y: linear, z: quadratic, w: radius
};

// Fixed-size array inside a Uniform Buffer
layout(std140, binding = 0) uniform gllightbuffer_t {
    pointlight_t lights[16];
    vec3 global_ambient;
    float global_specular_strength;
    int numLights;
};

struct material_t {
    vec4 color;
};

uniform material_t  material;
uniform vec3        camerapos;

void main()
{
    //FragColor = material.color; 

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(camerapos - FragPos);

    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    for (int i = 0; i < numLights; ++i)
    {
        vec3 lightPos   = lights[i].position.xyz;
        vec3 lightColor = lights[i].color.rgb * lights[i].color.a; // RGB * Intensity

        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);

        // Distance & Attenuation
        float dist = length(lightPos - FragPos);

        // Ensure denominator is at least 0.0001 to prevent division by zero
        float denom = lights[i].params.x + lights[i].params.y * dist + lights[i].params.z * (dist * dist);
        float atten = 1.0 / max(denom, 0.0001);

        // Blinn-Phong Specular (Half-way vector)
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfDir), 0.0), 32.0); // 32.0 = shininess

        // Both diffuse and specular attenuate with distance
        totalDiffuse  += lightColor * diff * atten;
        totalSpecular += lightColor * (spec * global_specular_strength) * atten;
    }

    // Material color tints ambient and diffuse; specular stays colored by the light source
    vec3 ambientResult = material.color.rgb * global_ambient;
    vec3 diffuseResult = material.color.rgb * totalDiffuse;
    vec3 specularResult = totalSpecular;

    vec3 finalColor = ambientResult + diffuseResult + specularResult;

    // Simple Reinhard Tone Mapping to prevent bright light blowout above 1.0
    finalColor = finalColor / (finalColor + vec3(1.0));

    FragColor = vec4(finalColor, material.color.a);
}
