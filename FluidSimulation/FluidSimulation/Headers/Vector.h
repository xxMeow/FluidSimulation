#pragma once

#include <math.h>

struct Vec2
{
    double x;
    double y;
    
    Vec2() : x(0), y(0) { }
    Vec2(double x0, double y0) : x(x0), y(y0) { }
    ~Vec2() { }

    void operator=(Vec2 v)
    {
        x = v.x;
        y = v.y;
    }
    Vec2 operator+(Vec2 v) { return Vec2(x+v.x, y+v.y); }
    Vec2 operator-(Vec2 v) { return Vec2(x-v.x, y-v.y); }
    Vec2 operator*(double n) { return Vec2(x*n, y*n); }
    Vec2 operator/(double n) { return Vec2(x/n, y/n); }
    bool operator==(Vec2 &v) { return x == v.x && y == v.y; }
    bool operator!=(Vec2 &v) { return x != v.x || y != v.y; }
    Vec2 &operator+=(Vec2 v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }
    Vec2 &operator-=(Vec2 v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    double len() { return sqrt(x*x + y*y); }
    double dst(Vec2 v) { return sqrt(pow(x-v.x, 2) + pow(y-v.y, 2)); }
};

struct Vec3
{
    double x;
    double y;
    double z;
    
    Vec3() : x(0), y(0), z(0) { }
    Vec3(double x0, double y0, double z0) : x(x0), y(y0), z(z0) { }
    ~Vec3() {}
    
    void operator=(Vec3 v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }
    Vec3 operator+(Vec3 v) { return Vec3(x+v.x, y+v.y, z+v.z); }
    Vec3 operator-(Vec3 v) { return Vec3(x-v.x, y-v.y, z-v.z); }
    Vec3 operator*(double n) { return Vec3(x*n, y*n, z*n); }
    Vec3 operator/(double n) { return Vec3(x/n, y/n, z/n); }
    bool operator==(Vec3 &v) { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(Vec3 &v) { return x != v.x || y != v.y || z != v.z; }
    Vec3 &operator+=(Vec3 v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    Vec3 &operator-=(Vec3 v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    friend Vec3 operator*(double n, Vec3 v)
    {
        v.x *= n;
        v.y *= n;
        v.z *= n;
        return v;
    }
    friend Vec3 operator/(double n, Vec3 v)
    {
        v.x /= n;
        v.y /= n;
        v.z /= n;
        return v;
    }

    double len() { return sqrt(x*x + y*y + z*z); }
    double dst(Vec3 v) { return sqrt(pow(x-v.x, 2) + pow(y-v.y, 2) + pow(z-v.z, 2)); }
    void nor()
    {
        double w = len();
        if (w < 0.00001) return;
        x /= w;
        y /= w;
        z /= w;
    }
    
    static double Dot(Vec3 v1, Vec3 v2) { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }
    static Vec3 Cross(Vec3 v1, Vec3 v2) { return Vec3(v1.y*v2.z-v1.z*v2.y, v1.z*v2.x-v1.x*v2.z, v1.x*v2.y-v1.y*v2.x); }
};
