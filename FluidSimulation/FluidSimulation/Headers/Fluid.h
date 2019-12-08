#pragma once

#include <vector>
#include <iostream>

#include <math.h>

#include "Point.h"
#include "Rigid.h"

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
        
        printf("Boundary:\n");
        printf("\tx(%f, %f) -> %f\n", xMin, xMax, size.x);
        printf("\ty(%f, %f) -> %f\n", yMin, yMax, size.y);
        printf("\tz(%f, %f) -> %f\n", zMin, zMax, size.z);
    }
    ~Boundary() { }
};

class Fluid
{
private:
    const int resolution = 2; // TODO: Renaming - resolution? particleRadius?
    const int iterationFreq = 10;
    const int gasConst = 50;
    const double restDensity = 8;
    const double viscosity = 0.8;
    const double kernelRadius = 1.0;
public:
    const int particleSize = 20;
    Boundary* boundary;
    Vec3 position; // Left bottom back point's init position of fluid cube
    Vec3 size; // Size of fluid cube
    std::vector<Particle*> particles;
    Vec3 gridSize;
    std::vector< std::vector< std::vector< std::vector<Particle*> > > > hashGrid;
    
public:
    Fluid(Boundary* boundary, Vec3 size, Vec3 posOffset, Vec3 initV) : boundary(boundary), size(size)
    {
        if (size.x <= 0 || size.y <= 0 || size.z <= 0) {
            std::cout << "Fluid size can't be negative." << std::endl;
            exit(-1);
        }
        if (posOffset.x < 0 || posOffset.y < 0 || posOffset.z < 0) {
            std::cout << "Fluid offset can't be negative." << std::endl;
            exit(-1);
        }
        
        // Initialize hash grids, each grid is a vector of Particle*
        gridSize = boundary->size;
        hashGrid.resize(gridSize.z);
        for (int i = 0; i < gridSize.z; i ++) {
            hashGrid[i].resize(gridSize.y);
            for (int j = 0; j < gridSize.y; j ++) {
                hashGrid[i][j].resize(gridSize.x);
            }
        }
        
        // Get the world coordinate of fluid
        position = boundary->position + posOffset;
        printf("Fluid:\n");
        printf("\tx(%f, %f)\n", position.x, position.x+size.x);
        printf("\ty(%f, %f)\n", position.y, position.y+size.y);
        printf("\tz(%f, %f)\n", position.z, position.z+size.z);
        if (position.x+size.x > boundary->xMax || position.y+size.y > boundary->yMax ||
            position.z+size.z > boundary->zMax)
        {
            std::cout << "Fluid out of boundary." << std::endl;
            exit(-1);
        }
        
        initParticles(initV);
        
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
    
    void update(float timestep, Vec3 gravity, Ball* ball)
    {
        makeHashTable();
        computeDensity();
        computeForce();
        integrate(timestep, gravity, ball);
    }

private:
    void initParticles(Vec3 initV)
    {
        int index = 0; // TODO: ?? necessery?
        double distInterval = 1.0 / resolution;
        printf("Particle Interval: %f\n", distInterval);
        for (double z = position.z; z < position.z+size.z; z += distInterval) {
            for (double y = position.y; y < position.y+size.y; y += distInterval) {
                for (double x = position.x; x < position.x+size.x; x += distInterval) {
                    Particle *p = new Particle(index, Vec3((x-boundary->position.x)/boundary->size.x*1.9, (y-boundary->position.y)/boundary->size.y/1.5, (z-boundary->position.z)/boundary->size.z/1.2), Vec3(x, y, z)); // TODO: set color
                    p->velocity = initV;
                    particles.push_back(p);
                    index ++;
                }
            }
        }
    }
    void makeHashTable() // TODO: how to slice grid? use boundary?
    {
        for (int i = 0; i < gridSize.z; i ++) {
            for (int j = 0; j < gridSize.y; j ++) {
                for (int k = 0; k < gridSize.x; k ++) {
                    hashGrid[i][j][k].clear();
                }
            }
        }
        
        // TODO: push to which grid??
        for (int i = 0; i < particles.size(); i ++)
        {
            Particle *p = particles[i];
            int gridX = (int)(p->position.x - boundary->position.x);
            int gridY = (int)(p->position.y - boundary->position.y);
            int gridZ = (int)(p->position.z - boundary->position.z);

            if (gridX < 0) gridX = 0;
            if (gridX >= gridSize.x) gridX = gridSize.x - 1;
            if (gridY < 0) gridY = 0;
            if (gridY >= gridSize.y) gridY = gridSize.y - 1;
            if (gridZ < 0) gridZ = 0;
            if (gridZ >= gridSize.z) gridZ = gridSize.z - 1;

            hashGrid[gridZ][gridY][gridX].push_back(p);
        }
    }
    std::vector<Particle *> getNeighbors(int gridZ, int gridY, int gridX, std::vector<Particle*>& mine, double radius)
    {
        std::vector<Particle *> neighbors;
        mine.clear();
        for (int i = gridZ - (int)radius; i <= gridZ + (int)radius; i ++) {
            for (int j = gridY - (int)radius; j <= gridY + (int)radius; j ++) {
                for (int k = gridX - (int)radius; k <= gridX + (int)radius; k ++) {
                    if (i < 0 || i >= gridSize.z || j < 0 || j >= gridSize.y || k < 0 || k >= gridSize.x)
                        continue;

                    for (int index = 0; index < hashGrid[i][j][k].size(); index ++) {
                        neighbors.push_back(hashGrid[i][j][k][index]);

                        if (i == gridZ && j == gridY && k == gridX) { mine.push_back(hashGrid[i][j][k][index]);
                        }
                    }
                }
            }
        }
        return neighbors;
    }
    void computeDensity()
    {
        for (int z = 0; z < gridSize.z; z ++) {
            for (int y = 0; y < gridSize.y; y ++) {
                for (int x = 0; x < gridSize.x; x ++) {
                    std::vector<Particle*> mine;
                    std::vector<Particle*> neighbors = getNeighbors(z, y, x, mine, kernelRadius);

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
        for (int z = 0; z < gridSize.z; z ++) {
            for (int y = 0; y < gridSize.y; y ++) {
                for (int x = 0; x < gridSize.x; x ++) {
                    std::vector<Particle*> mine;
                    std::vector<Particle*> neighbors = getNeighbors(z, y, x, mine, kernelRadius);

                    for (int i = 0; i < mine.size(); i ++)
                    {
                        Particle *pi = mine[i];
                        pi->fPressure = Vec3(0, 0, 0);//compute with spikygradientKernel
                        pi->fViscosity = Vec3(0, 0, 0);//compute with viscositylaplacianKernel

                        for (int j = 0; j < neighbors.size(); j ++) {
                            double temp;
                            Particle* pj = neighbors[j];
                            Vec3 spikyValue = spikyGradientKernel(pi->position - pj->position);
                            temp = pj->mass * (gasConst * ((pi->density-restDensity) + pj->density - restDensity)) / (2.0 * pj->density);
                            pi->fPressure += spikyValue * temp;
                            double laplacValue = viscosityLaplacianKernel(pi->position - pj->position);
                            pi->fViscosity += pj->mass * ((pi->velocity - pj->velocity) / pj->density) * laplacValue;
                        }
                        
                        pi->fPressure = -1.0 * pi->fPressure;
                        pi->fViscosity = viscosity * pi->fViscosity;
                    }
                }
            }
        }
    }
    Vec3 getWorldPos(Particle* p) { return boundary->position + p->position; }
    void setWorldPos(Particle* p, Vec3 pos) { p->position = pos - boundary->position; }
    void integrate(double timestep, Vec3 gravity, Ball* ball)
    {
        for (int i = 0; i < particles.size(); i++)
        {
            Particle *p = particles[i];
            Vec3 fGravity = p->mass * gravity;
            // Update velocity and position
            p->acceleration = (p->fPressure + p->fViscosity) / p->density + fGravity;
            p->velocity += p->acceleration * timestep;
            p->position += p->velocity * timestep;
            
            /** Boundary Check **/
            {
                double pRadius = particleSize/90.0;
                if (p->position.x < boundary->xMin && p->velocity.x < 0.0)
                {
                    p->velocity.x *= -(p->restitution);
                    p->position.x = boundary->xMin+pRadius+0.1;
                }
                if (p->position.x > boundary->xMax && p->velocity.x > 0.0)
                {
                    p->velocity.x *= -(p->restitution);
                    p->position.x = boundary->xMax-pRadius-0.1;
                }
                if (p->position.y < boundary->yMin && p->velocity.y < 0.0)
                {
                    p->velocity.y *= -(p->restitution);
                    p->position.y = boundary->yMin+pRadius+0.1;
                }
                if (p->position.y > boundary->yMax && p->velocity.y > 0.0)
                {
                    p->velocity.y *= -(p->restitution);
                    p->position.y = boundary->yMax-pRadius-0.2;
                }
                if (p->position.z < boundary->zMin && p->velocity.z < 0.0)
                {
                    p->velocity.z *= -(p->restitution);
                    p->position.z = boundary->zMin+pRadius+0.1;
                }
                if (p->position.z > boundary->zMax && p->velocity.z > 0.0)
                {
                    p->velocity.z *= -(p->restitution);
                    p->position.z = boundary->zMax-pRadius-0.1;
                }
            }
            
            /** Collision check **/
            Vec3 distVec = getWorldPos(p) - ball->center;
            double distLen = distVec.len();
            double safeDist = (ball->radius + particleSize/100.0)*1.05;
            if (distLen < safeDist) {
                distVec.nor();
                setWorldPos(p, distVec*safeDist+ball->center);
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
