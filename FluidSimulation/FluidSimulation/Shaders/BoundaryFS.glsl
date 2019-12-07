#version 330 core

out vec4 color;

in vec3 position;
in vec3 normal;

uniform vec4 uniBoundaryColor;
uniform vec3 uniLightPos;
uniform vec3 uniLightColor;

void main()
{
    // Ambient
    float ambientStrength = 0.5f;
    vec3 ambient = ambientStrength * uniLightColor;

    // Diffuse
    vec3 lightDir = normalize(uniLightPos - position);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uniLightColor;

    vec3 objectColor = vec3(uniBoundaryColor.x, uniBoundaryColor.y, uniBoundaryColor.z);
    vec3 result = (ambient + diffuse) * objectColor;
    color = vec4(result, uniBoundaryColor.w);
}
