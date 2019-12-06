#pragma once

#include <vector>
#include <iostream>

#include <math.h>

#include "Particle.h"

struct Boundary
{
    Vec3 position;
    Vec3 size;
    
    double xMin, xMax;
    double yMin, yMax;
    double zMin, zMax;
    
    Boundary(Vec3 position, Vec3 size) : position(position), size(size) {
        if (size.x <= 0 || size.y <= 0 || size.z <= 0) {
            std::cout << "Boundary size can't be negative." << std::endl;
            exit(-1);
        }
        
        xMin = position.x;
        xMax = position.x + size.x;
        yMin = position.y;
        yMax = position.y + size.y;
        zMin = position.z;
        zMax = position.z + size.z;
    }
    ~Boundary() { }
};

class Fluid
{
private:
    const int resolution = 2; // TODO: Renaming - resolution? particleRadius?
    const int iterationFreq = 10;
    const int gasConst = 50;
    static const int gridSize = 40;
    static const int hashSize = 42; // = gridSize + 2
    const double restDensity = 4.2;
    const double viscosity = 0.8;
    const double kernelRadius = 1.0;
public:
    Boundary boundary;
    Vec3 position; // Left bottom back point's init position of fluid cube
    Vec3 size; // Size of fluid cube
    std::vector<Particle*> particles;
    std::vector<Particle*> hashGrid[gridSize][gridSize][gridSize];
    
public:
    Fluid(Boundary boundary, Vec3 size, Vec3 posOffset, Vec3 initV) : boundary(boundary), size(size)
    {
        if (size.x <= 0 || size.y <= 0 || size.z <= 0) {
            std::cout << "Fluid size can't be negative." << std::endl;
        }
        
        position = boundary.position + posOffset;
        if (position.x < boundary.xMin || position.x+size.x > boundary.xMax ||
            position.y < boundary.yMin || position.y+size.y > boundary.yMax ||
            position.z < boundary.zMin || position.z+size.z > boundary.zMax)
        {
            std::cout << "Fluid out of boundary." << std::endl;
        }
        
        init(initV);
        
        std::cout << "Fluid : " << particles.size() << " Paricles" << std::endl;
        printf("\t(%f, %f, %f)\n", particles[0]->position.x, particles[0]->position.y, particles[0]->position.z);
    }
    ~Fluid()
    {
        while (!particles.empty())
        {
            particles.pop_back();
        }
        particles.clear();
    }
    
    void update(float timestep, Vec3 gravity)
    {
        printf("Updating..\n");
        printf("\tMaking Hash Table..\n");
        makeHashTable();
        printf("\t(%f, %f, %f)\n", particles[0]->position.x, particles[0]->position.y, particles[0]->position.z);
        printf("\tComputing Density..\n");
        computeDensity();
        printf("\t(%f, %f, %f)\n", particles[0]->position.x, particles[0]->position.y, particles[0]->position.z);
        printf("\tComputing Force..\n");
        computeForce();
        printf("\t(%f, %f, %f)\n", particles[0]->position.x, particles[0]->position.y, particles[0]->position.z);
        printf("\tIntegrating..\n");
        integrate(timestep, gravity);
        printf("\t(%f, %f, %f)\n", particles[0]->position.x, particles[0]->position.y, particles[0]->position.z);
    }

private:
    void init(Vec3 initV)
    {
        int index = 0;
        double distInterval = 1.0 / resolution;
        for (double z = position.z; z < position.z+size.z; z += distInterval) {
            for (double y = position.y; y < position.y+size.y; y += distInterval) {
                for (double x = position.x; x < position.x+size.x; x += distInterval) {
                    Particle *p = new Particle(index, Vec3(x, y, z));
                    p->velocity = initV;
                    particles.push_back(p);
                    index ++;
                }
            }
        }
    }
    
    std::vector<Particle *> getNeighbors(int gridX, int gridY, int gridZ, std::vector<Particle*>& mine, double radius)
    {
        std::vector<Particle *> neighbors;
        mine.clear();
        for (int i = gridX - (int)radius; i <= gridX + (int)radius; i ++) {
            for (int j = gridY - (int)radius; j <= gridY + (int)radius; j ++) {
                for (int k = gridZ - (int)radius; k <= gridZ + (int)radius; k ++) {
                    if (i < 0 || i >= gridSize || j < 0 || j >= gridSize || k < 0 || k >= gridSize)
                        continue;

                    for (int index = 0; index < hashGrid[i][j][k].size(); index ++) {
                        neighbors.push_back(hashGrid[i][j][k][index]);

                        if (i == gridX && j == gridY && k == gridZ) { mine.push_back(hashGrid[i][j][k][index]);
                        }
                    }
                }
            }
        }
        return neighbors;
    }
    void makeHashTable() // TODO: how to slice grid? use boundary?
    {
        for (int i = 0; i < gridSize; i ++) {
            for (int j = 0; j < gridSize; j ++) {
                for (int k = 0; k < gridSize; k ++) {
                    hashGrid[i][j][k].clear();
                }
            }
        }
        
        // TODO: push to which grid??
        for (int i = 0; i < particles.size(); i ++)
        {
            Particle *p = particles[i];
            double x = (p->position.x - boundary.xMin) / boundary.size.x * gridSize;
            double y = (p->position.y - boundary.yMin) / boundary.size.y * gridSize;
            double z = (p->position.z - boundary.zMin) / boundary.size.z * gridSize;
            int gridX = (int)x;
            int gridY = (int)y;
            int gridZ = (int)z;

            if (gridX < 0) gridX = 0;
            if (gridX >= gridSize) gridX = gridSize - 1;
            if (gridY < 0) gridY = 0;
            if (gridY >= gridSize) gridY = gridSize - 1;
            if (gridZ < 0) gridZ = 0;
            if (gridZ >= gridSize) gridZ = gridSize - 1;

            hashGrid[gridX][gridY][gridZ].push_back(p);
        }
    }
    void computeDensity()
    {
        for (int x = 0; x < gridSize; x ++) {
            for (int y = 0; y < gridSize; y ++) {
                for (int z = 0; z < gridSize; z ++) {
                    std::vector<Particle*> mine;
                    std::vector<Particle*> neighbors = getNeighbors(x, y, z, mine, kernelRadius);

                    for (int i = 0; i < mine.size(); i++)
                    {
                        Particle* pi = mine[i];
                        pi->density = 0;
                        
                        for (int j = 0; j < neighbors.size(); j ++) {
                            Particle* pj = neighbors[j];
                            pi->density += pj->mass * poly6Kernel(pi->position - pj->position);
                        }
                    }
                }
            }
        }
    }
    void computeForce()
    {
        for (int x = 0; x < gridSize; x ++) {
            for (int y = 0; y < gridSize; y ++) {
                for (int z = 0; z < gridSize; z ++) {
                    std::vector<Particle*> mine;
                    std::vector<Particle*> neighbors = getNeighbors(x, y, z, mine, kernelRadius);

                    for (int i = 0; i < mine.size(); i ++)
                    {
                        Particle *pi = mine[i];
                        pi->fPressure = Vec3(0, 0, 0);//compute with spikygradientKernel
                        pi->fViscosity = Vec3(0, 0, 0);//compute with viscositylaplacianKernel

                        for (int j = 0; j < neighbors.size(); j ++) {
                            Particle* pj = neighbors[j];
                            pi->fPressure += pj->mass * (gasConst * ((pi->density-restDensity) + pj->density-restDensity)) / (2.0 * pj->density) * spikyGradientKernel(pi->position - pj->position);
                            pi->fViscosity += pj->mass * ((pi->velocity - pj->velocity) / pj->density) * viscosityLaplacianKernel(pi->position - pj->position);
                        }
                        
                        pi->fPressure = -1.0 * pi->fPressure;
                        pi->fViscosity = viscosity * pi->fViscosity;
                    }
                }
            }
        }
    }
    void integrate(double timestep, Vec3 gravity)
    {
        // Particle bondary check
        for (int i = 0; i < particles.size(); i++)
        {
            Particle *p = particles[i];
            
            Vec3 fGravity = p->mass * gravity;
            // Update velocity and position
            p->acceleration = (p->fPressure + p->fViscosity) / p->density + fGravity;
            p->velocity = p->velocity + p->acceleration * timestep;
            p->position = p->position + p->velocity * timestep;

            // Boundary condition
            if (p->position.x < boundary.xMin && p->velocity.x < 0.0)
            {
                p->velocity.x *= -(p->restitution);
                p->position.x = boundary.xMin+0.1;
            }
            if (p->position.x > boundary.xMax && p->velocity.x > 0.0)
            {
                p->velocity.x *= -(p->restitution);
                p->position.x = boundary.xMax-0.1;
            }
            if (p->position.y < boundary.yMin && p->velocity.y < 0.0)
            {
                p->velocity.y *= -(p->restitution);
                p->position.y = boundary.yMin+0.1;
            }
            if (p->position.y > boundary.yMax && p->velocity.y > 0.0)
            {
                p->velocity.y *= -(p->restitution);
                p->position.y = boundary.yMax-0.1;
            }
            if (p->position.z < boundary.zMin && p->velocity.z < 0.0)
            {
                p->velocity.z *= -(p->restitution);
                p->position.z = boundary.zMin+0.1;
            }
            if (p->position.z > boundary.zMax && p->velocity.z > 0.0)
            {
                p->velocity.z *= -(p->restitution);
                p->position.z = boundary.zMax-0.1;
            }
        }
    }

private: //kernel functions for SPH
    double poly6Kernel(Vec3 diffVec)
    {
        static double kernelRadius9 = pow(kernelRadius, 9);
        double temp = pow(kernelRadius, 2) - pow(diffVec.len(), 2);
        if (temp <= 0) {
            return 0;
        } else {
            return 315.0 / (64.0 * M_PI * kernelRadius9) * pow(temp, 3);
        }
    }
    Vec3 spikyGradientKernel(Vec3 diffVec)
    {
        static double kernelRadius9 = pow(kernelRadius, 9);
        double temp = pow(kernelRadius, 2) - pow(diffVec.len(), 2);
        if (temp <= 0) {
            return Vec3(0, 0, 0);
        } else {
            return diffVec * (-945.0 / (32.0 * M_PI * kernelRadius9) * pow(temp, 2));
        }
    }
    double viscosityLaplacianKernel(Vec3 diffVec)
    {
        static double kernelRadius9 = pow(kernelRadius, 9);
        double diffDist = diffVec.len();
        if (diffDist > kernelRadius) {
            return 0;
        } else {
            double temp1 = pow(kernelRadius, 2);
            double temp2 = pow(diffDist, 2);
            return -945.0 / (32.0 * M_PI * kernelRadius9) * (temp1-temp2) * (3*temp1 - 7*temp2);
        }
    }
};
