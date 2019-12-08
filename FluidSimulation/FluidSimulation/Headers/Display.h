#pragma once

#include <iostream>

#include "Vertex.h"
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
    glm::vec3 color = glm::vec3(1, 1, 1);
};
Light sun;

class BoundaryRender
{
    // TODO: set each render point as const
    const Boundary* boundary;
    
    std::vector<Vertex*> lines;
    int numLines;
    
    glm::vec4 uniBoundaryColor;
    
    glm::vec3 *vboPos; // Position
    glm::vec3 *vboNor; // Normal

    GLuint programID;
    GLuint vaoID;
    GLuint vboIDs[2];
    
    GLint aPtrPos;
    GLint aPtrNor;

public:
    BoundaryRender(Boundary* boundary)
    {
        Vertex v1(Vec3(boundary->xMin, boundary->yMin, boundary->zMin));
        Vertex v2(Vec3(boundary->xMax, boundary->yMin, boundary->zMin));
        Vertex v3(Vec3(boundary->xMax, boundary->yMin, boundary->zMax));
        Vertex v4(Vec3(boundary->xMin, boundary->yMin, boundary->zMax));
        Vertex v5(Vec3(boundary->xMin, boundary->yMax, boundary->zMin));
        Vertex v6(Vec3(boundary->xMax, boundary->yMax, boundary->zMin));
        Vertex v7(Vec3(boundary->xMax, boundary->yMax, boundary->zMax));
        Vertex v8(Vec3(boundary->xMin, boundary->yMax, boundary->zMax));
        
        // Bottom
        lines.push_back(&v1);
        lines.push_back(&v2);
        
        lines.push_back(&v2);
        lines.push_back(&v3);
        
        lines.push_back(&v3);
        lines.push_back(&v4);
        
        lines.push_back(&v4);
        lines.push_back(&v1);
        
        // Top
        lines.push_back(&v5);
        lines.push_back(&v6);
        
        lines.push_back(&v6);
        lines.push_back(&v7);
        
        lines.push_back(&v7);
        lines.push_back(&v8);
        
        lines.push_back(&v8);
        lines.push_back(&v5);
        
        // Middle
        lines.push_back(&v1);
        lines.push_back(&v5);
        
        lines.push_back(&v2);
        lines.push_back(&v6);
        
        lines.push_back(&v3);
        lines.push_back(&v7);
        
        lines.push_back(&v4);
        lines.push_back(&v8);
        
        numLines = (int)(lines.size()) / 2;
        if (numLines <= 0) {
            std::cout << "ERROR::BoundaryRender : No boundary exists." << std::endl;
            exit(-1);
        }
        
        glm::vec4 c(1, 1, 1, 1);
        glm::vec3 modelVec(boundary->position.x, boundary->position.y, boundary->position.z);
        
        uniBoundaryColor = c;
        
        vboPos = new glm::vec3[numLines*2];
        vboNor = new glm::vec3[numLines*2];
        for (int i = 0; i < numLines; i ++) {
            vboPos[i*2] = glm::vec3(lines[i*2]->position.x, lines[i*2]->position.y, lines[i*2]->position.z);
            vboPos[i*2+1] = glm::vec3(lines[i*2+1]->position.x, lines[i*2+1]->position.y, lines[i*2+1]->position.z);
            vboNor[i*2] = glm::vec3(lines[i*2]->normal.x, lines[i*2]->normal.y, lines[i*2]->normal.z);
            vboNor[i*2+1] = glm::vec3(lines[i*2]->normal.x, lines[i*2]->normal.y, lines[i*2]->normal.z);
        }
        
        /** Build render program **/
        Program program("Shaders/BoundaryVS.glsl", "Shaders/BoundaryFS.glsl");
        programID = program.ID;
        std::cout << "Boundary Program ID: " << programID << std::endl;

        // Generate ID of VAO and VBOs
        glGenVertexArrays(1, &vaoID);
        glGenBuffers(2, vboIDs);
        
        // Attribute pointers of VAO
        aPtrPos = 0;
        aPtrNor = 1;
        // Bind VAO
        glBindVertexArray(vaoID);
        
        // Position buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glVertexAttribPointer(aPtrPos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, numLines*2*sizeof(glm::vec3), vboPos, GL_DYNAMIC_DRAW);
        // Normal buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glVertexAttribPointer(aPtrNor, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, numLines*2*sizeof(glm::vec3), vboNor, GL_DYNAMIC_DRAW);
        
        // Enable it's attribute pointers since they were set well
        glEnableVertexAttribArray(aPtrPos);
        glEnableVertexAttribArray(aPtrNor);
        
        /** Set uniform **/
        glUseProgram(programID); // Active shader before set uniform
        // Set color
        glUniform4fv(glGetUniformLocation(programID, "uniBoundaryColor"), 1, &uniBoundaryColor[0]);
        
        /** Projection matrix : The frustum that camera observes **/
        // Since projection matrix rarely changes, set it outside the rendering loop for only onec time
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniProjMatrix"), 1, GL_FALSE, &cam.uniProjMatrix[0][0]);
        
        /** Model Matrix : Put rigid into the world **/
        glm::mat4 uniModelMatrix = glm::mat4(1.0f);
        uniModelMatrix = glm::translate(uniModelMatrix, modelVec);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniModelMatrix"), 1, GL_FALSE, &uniModelMatrix[0][0]);
        
        /** Light **/
        glUniform3fv(glGetUniformLocation(programID, "uniLightPos"), 1, &(sun.pos[0]));
        glUniform3fv(glGetUniformLocation(programID, "uniLightColor"), 1, &(sun.color[0]));

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbined VBO
        glBindVertexArray(0); // Unbined VAO
    }
    
    ~BoundaryRender()
    {
        delete [] vboPos;
        delete [] vboNor;
        
        if (vaoID)
        {
            glDeleteVertexArrays(1, &vaoID);
            glDeleteBuffers(2, vboIDs);
            vaoID = 0;
        }
        if (programID)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }
    
    void flush()
    {
        glUseProgram(programID);
        
        glBindVertexArray(vaoID);
        
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, numLines*2*sizeof(glm::vec3), vboPos);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, numLines*2*sizeof(glm::vec3), vboNor);
        
        /** View Matrix : The camera **/
        cam.uniViewMatrix = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniViewMatrix"), 1, GL_FALSE, &cam.uniViewMatrix[0][0]);
        
        glEnable(GL_BLEND);
//        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_ONE, GL_ONE);
        
        /** Draw **/
        glDrawArrays(GL_LINES, 0, numLines*2);
        
        // End flushing
        glDisable(GL_BLEND);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};

class FluidRender
{
    const Fluid* fluid;
    int numParticles;
    
    glm::vec3 *vboPos; // Position
    
    GLuint programID;
    GLuint vaoID;
    GLuint vboIDs[1];
    
    GLint aPtrPos;
    
public:
    FluidRender(Fluid* fluid)
    {
        numParticles = (int)(fluid->particles.size());
        if (numParticles <= 0) {
            // TODO: Formatting all the print out
            std::cout << "ERROR::FluidRender : No particles exists." << std::endl;
            exit(-1);
        }
        
        this->fluid = fluid;
        
        vboPos = new glm::vec3[numParticles];
        for (int i = 0; i < numParticles; i ++) {
            Particle* p = fluid->particles[i];
            vboPos[i] = glm::vec3(p->position.x, p->position.y, p->position.z);
        }
        
        /** Build render program **/
        Program program("Shaders/FluidVS.glsl", "Shaders/FluidFS.glsl");
        programID = program.ID;
        std::cout << "Fluid Program ID: " << programID << std::endl;

        // Generate ID of VAO and VBOs
        glGenVertexArrays(1, &vaoID);
        glGenBuffers(1, vboIDs);
        
        // Attribute pointers of VAO
        aPtrPos = 0;
        // Bind VAO
        glBindVertexArray(vaoID);
        
        // Position buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glVertexAttribPointer(aPtrPos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, numParticles*sizeof(glm::vec3), vboPos, GL_DYNAMIC_DRAW);
        
        // Enable it's attribute pointers since they were set well
        glEnableVertexAttribArray(aPtrPos);
        
        /** Set uniform **/
        glUseProgram(programID); // Active shader before set uniform
        
        /** Projection matrix : The frustum that camera observes **/
        // Since projection matrix rarely changes, set it outside the rendering loop for only onec time
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniProjMatrix"), 1, GL_FALSE, &cam.uniProjMatrix[0][0]);
        
        /** Model Matrix : Put cloth into the world **/
        glm::mat4 uniModelMatrix = glm::mat4(1.0f);
        uniModelMatrix = glm::translate(uniModelMatrix, glm::vec3(fluid->boundary->position.x, fluid->boundary->position.y, fluid->boundary->position.z));
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniModelMatrix"), 1, GL_FALSE, &uniModelMatrix[0][0]);
        
        /** Light **/
        glUniform3fv(glGetUniformLocation(programID, "uniLightPos"), 1, &(sun.pos[0]));
        glUniform3fv(glGetUniformLocation(programID, "uniLightColor"), 1, &(sun.color[0]));

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbined VBO
        glBindVertexArray(0); // Unbined VAO
    }
    ~FluidRender()
    {
        delete [] vboPos;
        
        if (vaoID)
        {
            glDeleteVertexArrays(1, &vaoID);
            glDeleteBuffers(1, vboIDs);
            vaoID = 0;
        }
        if (programID)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }
    
    void flush()
    {
        for (int i = 0; i < numParticles; i ++) {
            Particle* p = fluid->particles[i];
            vboPos[i] = glm::vec3(p->position.x, p->position.y, p->position.z);
        }
        
        glUseProgram(programID);
        
        glBindVertexArray(vaoID);
        
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, numParticles*sizeof(glm::vec3), vboPos);
        
        /** View Matrix : The camera **/
        cam.uniViewMatrix = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniViewMatrix"), 1, GL_FALSE, &cam.uniViewMatrix[0][0]);
        
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // TODO: which blend mode?
//        glBlendFunc(GL_ONE, GL_ONE);
        
        /** Draw **/
        glPointSize(100);
        glDrawArrays(GL_POINTS, 0, numParticles);
        
        // End flushing
        glDisable(GL_BLEND);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};

class RigidRender // Single color & Lighting
{
    std::vector<Vertex*> faces;
    int vertexCount; // Number of nodes in faces
    
    glm::vec4 uniRigidColor;
    
    glm::vec3 *vboPos; // Position
    glm::vec3 *vboNor; // Normal

    GLuint programID;
    GLuint vaoID;
    GLuint vboIDs[2];
    
    GLint aPtrPos;
    GLint aPtrNor;
    
public:
    RigidRender(std::vector<Vertex*> f, glm::vec4 c, glm::vec3 modelVec)
    {
        faces = f;
        vertexCount = (int)(faces.size());
        if (vertexCount <= 0) {
            std::cout << "ERROR::RigidRender : No vertex exists." << std::endl;
            exit(-1);
        }
        
        uniRigidColor = c;
        
        vboPos = new glm::vec3[vertexCount];
        vboNor = new glm::vec3[vertexCount];
        for (int i = 0; i < vertexCount; i ++) {
            Vertex* v = faces[i];
            vboPos[i] = glm::vec3(v->position.x, v->position.y, v->position.z);
            vboNor[i] = glm::vec3(v->normal.x, v->normal.y, v->normal.z);
        }
        
        /** Build render program **/
        Program program("Shaders/RigidVS.glsl", "Shaders/RigidFS.glsl");
        programID = program.ID;
        std::cout << "Rigid Program ID: " << programID << std::endl;

        // Generate ID of VAO and VBOs
        glGenVertexArrays(1, &vaoID);
        glGenBuffers(2, vboIDs);
        
        // Attribute pointers of VAO
        aPtrPos = 0;
        aPtrNor = 1;
        // Bind VAO
        glBindVertexArray(vaoID);
        
        // Position buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glVertexAttribPointer(aPtrPos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(glm::vec3), vboPos, GL_DYNAMIC_DRAW);
        // Normal buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glVertexAttribPointer(aPtrNor, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(glm::vec3), vboNor, GL_DYNAMIC_DRAW);
        
        // Enable it's attribute pointers since they were set well
        glEnableVertexAttribArray(aPtrPos);
        glEnableVertexAttribArray(aPtrNor);
        
        /** Set uniform **/
        glUseProgram(programID); // Active shader before set uniform
        // Set color
        glUniform4fv(glGetUniformLocation(programID, "uniRigidColor"), 1, &uniRigidColor[0]);
        
        /** Projection matrix : The frustum that camera observes **/
        // Since projection matrix rarely changes, set it outside the rendering loop for only onec time
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniProjMatrix"), 1, GL_FALSE, &cam.uniProjMatrix[0][0]);
        
        /** Model Matrix : Put rigid into the world **/
        glm::mat4 uniModelMatrix = glm::mat4(1.0f);
        uniModelMatrix = glm::translate(uniModelMatrix, modelVec);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniModelMatrix"), 1, GL_FALSE, &uniModelMatrix[0][0]);
        
        /** Light **/
        glUniform3fv(glGetUniformLocation(programID, "uniLightPos"), 1, &(sun.pos[0]));
        glUniform3fv(glGetUniformLocation(programID, "uniLightColor"), 1, &(sun.color[0]));

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbined VBO
        glBindVertexArray(0); // Unbined VAO
    }
    ~RigidRender()
    {
        delete [] vboPos;
        delete [] vboNor;
        
        if (vaoID)
        {
            glDeleteVertexArrays(1, &vaoID);
            glDeleteBuffers(2, vboIDs);
            vaoID = 0;
        }
        if (programID)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }
    
    void flush() // Rigid does not move, thus do not update vertexes' data
    {
        glUseProgram(programID);
        
        glBindVertexArray(vaoID);
        
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount*sizeof(glm::vec3), vboPos);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount*sizeof(glm::vec3), vboNor);
        
        /** View Matrix : The camera **/
        cam.uniViewMatrix = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniViewMatrix"), 1, GL_FALSE, &cam.uniViewMatrix[0][0]);
        
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        /** Draw **/
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        
        // End flushing
        glDisable(GL_BLEND);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};

struct Ground
{
    Vec3 position;
    int width, height;
    glm::vec4 color;
    const double friction = 0.9;
    
    std::vector<Vertex*> vertexes;
    std::vector<Vertex*> faces;
    
    Ground(Vec3 pos, Vec2 size, glm::vec4 c) {
        position = pos;
        width = size.x;
        height = size.y;
        color = c;
        
        init();
    }
    ~Ground()
    {
        for (int i = 0; i < vertexes.size(); i++) { delete vertexes[i]; }
        vertexes.clear();
        faces.clear();
    }
    
    void init()
    {
        vertexes.push_back(new Vertex(Vec3(0.0, 0.0, 0.0)));
        vertexes.push_back(new Vertex(Vec3(width, 0.0, 0.0)));
        vertexes.push_back(new Vertex(Vec3(0.0, 0.0, -height)));
        vertexes.push_back(new Vertex(Vec3(width, 0.0, -height)));
        
        for (int i = 0; i < vertexes.size(); i ++) {
            vertexes[i]->normal = Vec3(0.0, 1.0, 0.0); // It's not neccessery to normalize here
            
            // Debug info
            printf("Ground[%d]: (%f, %f, %f) - (%f, %f, %f)\n", i, vertexes[i]->position.x, vertexes[i]->position.y, vertexes[i]->position.z, vertexes[i]->normal.x, vertexes[i]->normal.y, vertexes[i]->normal.z);
        }
        
        faces.push_back(vertexes[0]);
        faces.push_back(vertexes[1]);
        faces.push_back(vertexes[2]);
        faces.push_back(vertexes[1]);
        faces.push_back(vertexes[2]);
        faces.push_back(vertexes[3]);
    }
};

class GroundRender
{
public:
    Ground *ground;
    RigidRender *render;
    
    GroundRender(Ground* g)
    {
        ground = g;
        render = new RigidRender(ground->faces, ground->color, glm::vec3(ground->position.x, ground->position.y, ground->position.z));
    }
    
    void flush() { render->flush(); }
};

class Sphere
{
public:
    const int meridianNum = 24;
    const int parallelNum = 250;
    
    int radius;
    
    std::vector<Vertex*> vertexes;
    std::vector<Vertex*> faces;
    
    Sphere(int r)
    {
        radius = r;
        init();
    }
    ~Sphere()
    {
        for (int i = 0; i < vertexes.size(); i++) { delete vertexes[i]; }
        vertexes.clear();
        faces.clear();
    }
    
    Vertex* getTop() { return vertexes[0]; }
    Vertex* getVertex(int x, int y)
    {
        if (x < 0 || x >= parallelNum || y < 0 || y >= meridianNum) {
            printf("Vertex Index Out of Range.\n");
            exit(-1);
        } else {
            return vertexes[1+x*meridianNum+y];
        }
    }
    Vertex* getBottom() { return vertexes[vertexes.size()-1]; }
    
    Vec3 computeFaceNormal(Vertex* v1, Vertex* v2, Vertex* v3)
    {
        return Vec3::Cross(v2->position - v1->position, v3->position - v1->position);
    }
    
    void computeSphereNormal()
    {
        Vec3 normal(0.0, 0.0, 0.0);
        for (int i = 0; i < vertexes.size(); i ++) {
            vertexes[i]->normal = normal;
        }
        
        // The normal of all faces of the first and last cycle should be calculated specially!
        for (int i = 0; i < faces.size()/3; i ++) {
            Vertex* v1 = faces[i*3+0];
            Vertex* v2 = faces[i*3+1];
            Vertex* v3 = faces[i*3+2];
            
            normal = computeFaceNormal(v1, v3, v2);
            v1->normal += normal;
            v2->normal += normal;
            v3->normal += normal;
        }
        
        for (int i = 0; i < vertexes.size(); i ++) {
            vertexes[i]->normal.nor();
        }
    }
    
    void init() // Initialize vertexes coord and slice faces
    {
        /** Compute vertex position **/
        double cycleInterval = radius*2.0 / (parallelNum+1);
        double radianInterval = 2.0*M_PI/meridianNum;
        
        
        Vec3 pos(0.0, radius, 0.0);
        vertexes.push_back(new Vertex(pos)); // Top vertex
        
        for (int i = 0; i < parallelNum; i ++) {
            pos.y -= cycleInterval;
            for (int j = 0; j < meridianNum; j ++) {
                double xzLen = radius * sqrt(1.0 - pow(pos.y/radius, 2));
                double xRadian = j * radianInterval;  // The length of projection line on X-Z pane
                
                pos.x = xzLen * sin(xRadian);
                pos.z = xzLen * cos(xRadian);
                vertexes.push_back(new Vertex(pos));
            }
        }
        pos = Vec3(0.0, -radius, 0.0);
        vertexes.push_back(new Vertex(pos)); // Bottom vertex
        
        /** Slice faces **/
        // Top cycle
        for (int i = 0; i < meridianNum; i ++) {
            faces.push_back(getVertex(0, i));                               //   *   //
            faces.push_back(getTop());                                      //  / \  //
            faces.push_back(getVertex(0, (i+1)%meridianNum));               // *---* //
        }
        // Middle cycles
        for (int i = 0; i < parallelNum-1; i ++) {
            for (int j = 0; j < meridianNum; j ++) {
                faces.push_back(getVertex(i, j));                           //  *--* //
                faces.push_back(getVertex(i, (j+1)%meridianNum));           //  | /  //
                faces.push_back(getVertex(i+1, j));                         //  *    //
                
                faces.push_back(getVertex(i+1, (j+1)%meridianNum));         //     * //
                faces.push_back(getVertex(i+1, j));                         //   / | //
                faces.push_back(getVertex(i, (j+1)%meridianNum));           //  *--* //
            }
        }
        // Bottom cycle
        for (int i = 0; i < meridianNum; i ++) {
            faces.push_back(getBottom());                                   // *---* //
            faces.push_back(getVertex(parallelNum-1, i));                   //  \ /  //
            faces.push_back(getVertex(parallelNum-1, (i+1)%meridianNum));   //   *   //
        }
        
        /** Set normals **/
        computeSphereNormal();
    }
};

struct Ball
{
    Vec3 center;
    int radius;
    glm::vec4 color;
    const double friction = 0.8;
    
    Sphere* sphere;
    
    Ball(Vec3 cen, int r, glm::vec4 c)
    {
        center = cen;
        radius = r;
        color = c;
        
        sphere = new Sphere(radius);
    }
    ~Ball() {}
};

struct BallRender
{
    Ball* ball;
    RigidRender* render;
    
    BallRender(Ball* b)
    {
        ball = b;
        render = new RigidRender(ball->sphere->faces, ball->color, glm::vec3(ball->center.x, ball->center.y, ball->center.z));
    }
    
    void flush() { render->flush(); }
};
