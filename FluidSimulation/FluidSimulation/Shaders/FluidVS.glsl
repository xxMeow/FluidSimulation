#version 330 core

layout (location = 0) in vec3 vsPosition;

uniform mat4 uniModelMatrix;
uniform mat4 uniViewMatrix;
uniform mat4 uniProjMatrix;

void main()
{
    gl_Position = uniProjMatrix * uniViewMatrix * uniModelMatrix * vec4(vsPosition, 1.0f);
}
