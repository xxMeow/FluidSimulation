#pragma once

#include "Vector.h"

struct Particle
{
    int     index;
    double  mass;
    double  density;
    double  restitution;
    Vec3    position;
    Vec3    velocity;
    Vec3    acceleration;
    Vec3    fPressure;
    Vec3    fViscosity;
    
    Particle() { }
    Particle(int index, Vec3 position) : index(index), position(position), velocity(0, 0, 0), acceleration(0, 0, 0), fPressure(0, 0, 0), fViscosity(0, 0, 0)
    {
        mass = 1.0;
        density = 0.0;
        restitution = 0.5;
    }
    ~Particle() { }
};
