#version 330 core

out vec4 color;

uniform vec3 uniLightPos;
uniform vec3 uniLightColor;

void main()
{
    color = vec4(1, 1, 1, 1.0f);
}
