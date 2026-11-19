#pragma once

struct Coordinates3d
{
    int x, y, z;

    Coordinates3d(int _x, int _y, int _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    Coordinates3d() : x(0), y(0), z(0) {}

    void setx(int _x) { x = _x; }
    void sety(int _y) { y = _y; }
    void setz(int _z) { z = _z; }

    bool operator<(const Coordinates3d& other) const
    {
        if (x != other.x)
            return x < other.x;
        if (y != other.y)
            return y < other.y;
        return z < other.z;
    }
};