#pragma once

#include <iostream>

#include "Fluid.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Program.h"

struct Camera
{
    const float speed = 0.05f;
    const float frustumRatio = 1.0f;
    
    glm::vec3 pos = glm::vec3(0.0f, 4.0f, 12.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -2.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    glm::mat4 uniProjMatrix;
    glm::mat4 uniViewMatrix;
    
    Camera()
    {
        /** Projection matrix : The frustum that camera observes **/
        uniProjMatrix = glm::mat4(1.0f);
        uniProjMatrix = glm::perspective(glm::radians(45.0f), frustumRatio, 0.1f, 100.0f);
        /** View Matrix : The camera **/
        uniViewMatrix = glm::mat4(1.0f);
    }
};
Camera cam;

struct Light
{
    glm::vec3 pos = glm::vec3(-5.0f, 7.0f, 6.0f);
    glm::vec3 color = glm::vec3(0.7f, 0.7f, 1.0f);
};
Light sun;

class FluidRender
{
    const Fluid* fluid;
    
    FluidRender(Fluid* fluid)
    { }
    
    void destroy()
    { }
    
    void flush()
    { }
};
