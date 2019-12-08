#version 330 core

layout (location = 0) in vec3 vsPosition;
layout (location = 1) in vec3 vsColor;

out vec3 fsColor;

uniform mat4 uniModelMatrix;
uniform mat4 uniViewMatrix;
uniform mat4 uniProjMatrix;

void main()
{
    fsColor = vsColor;
    gl_Position = uniProjMatrix * uniViewMatrix * uniModelMatrix * vec4(vsPosition, 1.0f);
}
