#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

#include "Headers/Fluid.h"
#include "Headers/Display.h"

#define WIDTH 800
#define HEIGHT 800

#define TIME_STEP 0.04

/** Functions **/
void processInput(GLFWwindow *window);

/** Callback functions **/
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

/** Global **/
// Window and world
GLFWwindow *window;
glm::vec3 bgColor(200/255.0, 200/255.0, 200/255.0);
// Fluid
Boundary boundary(Vec3(-3.25, -3, -9), Vec3(13, 13, 3));
Vec3 fluidSize(3, 13, 3);
Vec3 fluidPosOffset(0, 0, 0);
Vec3 fluidInitVelocity(0, 0, 0);
Fluid fluid(&boundary, fluidSize, fluidPosOffset, fluidInitVelocity);
Vec3 gravity(0, -1, 0);
// Ground
Vec3 groundPos(-10, -6.5, -9);
Vec2 groundSize(20, 20);
glm::vec4 groundColor(16/255.0, 176/255.0, 202/255.0, 0.3);
Ground ground(groundPos, groundSize, groundColor);
// Ball
Vec3 ballPos(5, -1, -16.5);
int ballRadius = 1;
glm::vec4 ballColor(70/255.0, 70/255.0, 200/255.0, 1.0f);
Ball ball(ballPos, ballRadius, ballColor);

int main(int argc, const char * argv[])
{
    /** Prepare for rendering **/
    // Initialize GLFW
    glfwInit();
    // Set OpenGL version number as 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Use the core profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // MacOS is forward compatible
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    /** Create a GLFW window **/
    window = glfwCreateWindow(WIDTH, HEIGHT, "Fluid Simulation", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set the context of this window as the main context of current thread
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD : this should be done before using any openGL function
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD." << std::endl;
        glfwTerminate(); // This line isn't in the official source code, but I think that it should be added here.
        return -1;
    }
    
    /** Register callback functions **/
    // Callback functions should be registered after creating window and before initializing render loop
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    /** Renderers **/
    // Render program definitions
    GroundRender groundRender(&ground);
    FluidRender fluidRender(&fluid);
    BoundaryRender boundaryRender(&boundary);
    BallRender ballRender(&ball);
    
    glEnable(GL_DEPTH_TEST);
    
    /** Redering loop **/
    while (!glfwWindowShouldClose(window))
    {
        /** Check for events **/
        processInput(window);
        
        /** Set background clolor **/
        glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0); // Set color value (R,G,B,A) - Set Status
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        /** -------------------------------- Simulation & Rendering -------------------------------- **/
        
        fluid.update(TIME_STEP, gravity, &ball);
        groundRender.flush();
        fluidRender.flush();
        boundaryRender.flush();
        ballRender.flush();
        
        /** -------------------------------- Simulation & Rendering -------------------------------- **/
        
        glfwSwapBuffers(window);
        glfwPollEvents(); // Update the status of window
    }

    glfwTerminate();
    
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    /** Keyboard control **/ // If key did not get pressed it will return GLFW_RELEASE
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        cam.pos = glm::vec3(-14.0f, 10.0f, 1.0f);
        cam.front = glm::vec3(1.5f, -1.0f, -2.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        cam.pos = glm::vec3(17.0f, 13.0f, -12.0f);
        cam.front = glm::vec3(-6.0f, -4.7f, -2.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        cam.pos = glm::vec3(0.0f, 4.0f, 15.0f);
        cam.front = glm::vec3(0.0f, 0.0f, -2.0f);
    }
    
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cam.front.x += cam.speed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cam.front.x -= cam.speed;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cam.front.y += cam.speed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        cam.front.y -= cam.speed;
    }
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.pos.y += cam.speed*2;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.pos.y -= cam.speed*2;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.pos.x -= cam.speed*2;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.pos.x += cam.speed*2;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        cam.pos.z -= cam.speed*2;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cam.pos.z += cam.speed*2;
    }
    
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        printf("Camera:\n");
        printf("\tPos: %f, %f, %f\n", cam.pos.x, cam.pos.y, cam.pos.z);
        printf("\tFront %f, %f, %f\n", cam.front.x, cam.front.y, cam.front.z);
    }
}
