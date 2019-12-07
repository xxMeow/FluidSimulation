#pragma once

#include "Vector.h"

struct Vertex
{
public:
    Vec3 position;
    Vec3 normal;
    
    Vertex() {}
    Vertex(Vec3 pos)
    {
        position = pos;
    }
    ~Vertex() {}
};
