#pragma once
#include "Types.h"

struct Coordinates2d
{
    ll x,y;
    Coordinates2d(ll _x, ll _y)
    {
        x = _x;
        y = _y;
    }
    Coordinates2d(){}

    void setx(ll _x)
    {
        x = _x;
    }
    void sety(ll _y)
    {
        y = _y;
    }
    bool operator<(const Coordinates2d& other) const
    {
        if (x != other.x)
            return x < other.x;
        return y < other.y;
    }
};
