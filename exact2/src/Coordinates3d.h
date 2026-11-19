#pragma once
#include "Types.h"

struct Coordinates3d
{
    ll x, y, z;

    Coordinates3d(ll _x, ll _y, ll _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    Coordinates3d() : x(0), y(0), z(0) {}

    void setx(ll _x) { x = _x; }
    void sety(ll _y) { y = _y; }
    void setz(ll _z) { z = _z; }

    bool operator<(const Coordinates3d& other) const
    {
        if (x != other.x)
            return x < other.x;
        if (y != other.y)
            return y < other.y;
        return z < other.z;
    }
};