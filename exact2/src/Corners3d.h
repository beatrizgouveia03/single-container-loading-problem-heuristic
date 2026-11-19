#pragma once

#include <vector>
#include "Package.h"
#include "Coordinates3d.h"

using namespace std;

class Corners3d
{
public:
    public:
    static vector<Coordinates3d> getValidCorners(vector<Package> insertedPackages,
                                                 vector<Package> Packages,
                                                 int W, int H, int D);
};