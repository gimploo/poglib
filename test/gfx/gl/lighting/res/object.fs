#version 330 core

struct SimpleMaterial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct ComplexMaterial {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform SimpleMaterial  smaterial;
uniform ComplexMaterial cmaterial;
uniform Light           light;

void main()
{
    vec3 objectColor = vec3(1.0, 0.5, 0.31);

    // ambient
    /*vec3 ambient = smaterial.ambient * light.ambient;*/
    vec3 ambient = vec3(texture(cmaterial.diffuse, TexCoord)) * light.ambient;

    // diffuse
    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    /*vec3 diffuse  = diff * smaterial.diffuse * light.diffuse;*/
    vec3 diffuse  = (diff * vec3(texture(cmaterial.diffuse, TexCoord)) * light.diffuse;

    // specular
    vec3 viewDir              = normalize(-FragPos);
    vec3 reflectDir           = reflect(-lightDir, norm);
    float spec                = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    /*vec3 specular             = smaterial.specular * spec * light.specular;*/
    vec3 specular             = vec3(texture(cmaterial.specular, TexCoord)) * spec * light.specular;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
