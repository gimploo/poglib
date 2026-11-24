#version 410 core

// macOS downgrade of instance-frag.glsl.
// The 410 vertex shader does not emit lightspace_position, and the workbench
// never binds a shadow map for these editor primitives, so the shadow lookup
// is dropped here. `has_texture` is a float because the ECS uploads it with
// glUniform1f.

out vec4 FragColor;

in vec4 color;
in vec2 uv;
in vec3 FragPos;

uniform sampler2D prototypeTexture; // unit 0
uniform float has_texture;

void main()
{
    if (has_texture > 0.5) {
        FragColor = texture(prototypeTexture, uv) * color;
    } else {
        FragColor = color;
    }
}
